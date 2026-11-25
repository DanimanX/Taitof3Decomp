/* Darius Gaiden (Taito F3) — ROM 0x0C6A64 Task_PlayerControl (kernel slot 5),
 * the per-frame player lane, plus the player-pool helpers of 0x0C6B36..
 * 0x0C6E58 that belong to it: 0x0C6DEC (shot pool reset), 0x0C6CE6 (arm the
 * respawn tick), 0x0C6D0E (respawn), 0x0C6E10 (pick the frame's player),
 * 0x0C7D9A and 0x0C8488/0x0C87F8 (HUD animation rows).
 *
 * One call of PlayerControl_RunFrame() == one frame of the ROM task. The task
 * yields (TRAP #5) more than once per frame and, at a yield, where both players
 * are out it enters the stage-fail loop 0x0C6E5A which never returns: both are
 * modelled with `continue` on the same loop, so the per-yield side effects land
 * exactly where the ROM lands them.
 *
 * Callees that already live in src/ are called through their headers
 * (Obj_Spawn, ...). 0x0B9080 (object-script pass), 0x5566/0x5690/0x571A
 * (text), 0x1660 (palette block copy), 0x0C8AB4..0x0C8F48 (HUD digits),
 * 0x01016/0x01034/0x0103A/0x0CFEC (credit entry) and 0x13D638 (hi-score slot)
 * are NOT decompiled in src/: a frame that reaches one returns -1 *before* its
 * side effects, which is what the differential test counts.
 *
 * A5 = 0x408000. Struct map: docs/PLAYER.md. */
#include "player_task.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/palette.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ---- globals (A5-relative) -------------------------------------------- */
#define G_SCENE_STARTED (F3_A5 + 0x1D1Eu) /* bset.b: scene init done (Z = old) */
#define G_DEMO_MODE     (F3_A5 + 0x1D40u) /* word set to 0x1D by the init */
#define G_CTRL_FLAGS    (F3_A5 + 0x1D36u) /* b0..b4 lane flags: 0x08 no stick,
                                           * 0x10 shake (see Player_MoveAndBounds) */
#define G_FRAME_COUNTER (F3_A5 + 0x0878u) /* addq.w per yield (not per frame) */
#define G_PAUSE         (F3_A5 - 0x144Bu) /* bit 1 = paused (this lane idles) */
#define G_P2_DISABLED   (F3_A5 - 0x5DC6u) /* nonzero: P2 is not ticked */
#define G_ACTIVE_LIST   (F3_A5 + 0x1D4Cu) /* *(this) = P1 before the ticks */
#define G_FAIL_TIMER    (F3_A5 + 0x1D3Eu) /* 0x0C6E5A fail-text countdown */
#define G_BOMB_FLAGS    (F3_A5 + 0x1D58u) /* sf in the init */
#define G_LIVES_P1      (F3_A5 + 0x1D3Au) /* read only by 0x0C6CE6 */
#define G_LIVES_P2      (F3_A5 + 0x1D3Cu)
#define G_SHAKE_X       (F3_A5 + 0x1D20u) /* clr.l in the init */
#define G_SHAKE_Y       (F3_A5 + 0x1D24u)
#define G_DEMO_LEVEL    (F3_A5 + 0x1D4Au) /* clr.w in the init */
#define G_TIMER_ROW_1   (F3_A5 + 0x1D46u) /* 0x0C8488 HUD row index */
#define G_TIMER_CNT_1   (F3_A5 + 0x1D48u)
#define G_TIMER_ROW_2   (F3_A5 + 0x1D42u) /* 0x0C87F8 HUD row index */
#define G_TIMER_CNT_2   (F3_A5 + 0x1D44u)
#define G_ACTIVE_COUNT  (F3_A5 + 0x1D34u) /* players active this frame */
#define G_ACTIVE_OBJ    (F3_A5 + 0x1D28u) /* one of them (0x0C6E10) */
#define G_ACTIVE_TBL    (F3_A5 + 0x1D2Cu) /* their pointers */
#define G_PICK_SEED     (F3_A5 - 0x1456u) /* seed of the 0x0C6E10 pick */
#define G_SELECT_HOLD   (F3_A5 - 0x5DDFu) /* 0x0C6FBE: latched select/start */
#define G_COIN_BUSY     (F3_A5 - 0x5DC1u) /* 0x0C77BE/0x0C77E8 guard byte */
#define G_COIN_BUSY_2   (F3_A5 - 0x5DC3u)
#define P2_STATE_ADDR   (P1_STATE_ADDR + 0x8Au)

/* ---- fixed addresses --------------------------------------------------- */
#define SHIELD_STATE_ADDR 0x41B2A4u /* shield sprite state (0x0C7D9A) */
#define TEXT_STATE_P1     0x41B32Eu /* 0x0C84E4 text object, P1 */
#define TEXT_STATE_P2     0x41B392u /*                          P2 */
#define SHOT_POOL_BASE    0x410354u /* 64 shot objects of 0x0C6DEC */
#define SHOT_POOL_LAST    0x3Fu

/* ---- ROM tables -------------------------------------------------------- */
#define TBL_PAL_0C9242  0x0C9242u /* 0x001660 sources of the scene init */
#define TBL_FAIL_TEXT_A 0x0C94D6u /* 0x0C6E84: stage-fail labels */
#define TBL_FAIL_TEXT_B 0x0C9504u
#define TBL_FAIL_TEXT_C 0x0C9599u
#define TBL_SHOT_SCRIPT 0x0C95DEu /* fn of the respawn's Obj_Spawn call */
#define TBL_SPAWN_XY    0x0C926Eu /* s16 pairs per player_id (0x0C6D0E) */

/* ---- PlayerState fields (0x41B190 / 0x41B21A, 0x8A bytes) -------------- */
/* PS_POS_X/Y, PS_ANGLE, PS_HP, PS_BANK, PS_SPEED, PS_INVULN, PS_FLAGS come
 * from player_move.h (the move routine owns them); these are the rest. */
#define PS_PAL_BANK    0x0Cu
#define PS_TICK_FN     0x00u /* u32 fn: 0x0C7480 fail / 0x0C77BE ship / 0x0C793E respawn */
#define PS_PLAYER_ID   0x22u
#define PS_COLL_FLAGS  0x34u /* bit 0 = no contact damage */
#define PS_IO_CH       0x44u /* -> io_ch[n].cur (active-low nibble) */
#define PS_SCORE       0x48u
#define PS_SHOTS_LEFT  0x4Cu /* < 0 = player out */
#define PS_SHIELD_HP   0x4Eu
#define PS_AUX_52      0x52u
#define PS_MULT_SHOT   0x54u
#define PS_POWER_A     0x56u
#define PS_POWER_B     0x58u
#define PS_FIRE_COUNT  0x5Au
#define PS_WEAPON_A    0x5Cu
#define PS_SHIELD_TMR  0x5Eu
#define PS_AUX_60      0x60u
#define PS_CHILD_PTR   0x62u /* shot/child object handle */
#define PS_AUX_64      0x64u
#define PS_ARM_PAIR    0x6Au /* 0x0C6D0E re-arms {value,-1} at +0x6A and +0x7A */
#define PS_LIFE_PAIR   0x7Au
#define PS_DMG         0x28u
#define PS_X PS_POS_X /* aliases of the player_move.h names */
#define PS_Y PS_POS_Y

/* ---- helpers of this range -------------------------------------------- */

/* ======================================================================== */
/* ROM 0x0C6DEC: reset the 64 shot-pool objects (idle anim, no collision).   */
/* ======================================================================== */
void Player_ShotPoolReset(const f3mem_t *m)
{
    uint32_t a0 = SHOT_POOL_BASE;
    int16_t d0w = SHOT_POOL_LAST;
    do {
        wr32(m, a0 + 0x10u, 0x00028740u);
        wr16(m, a0 + 0x1Eu, 0u);
        wr8(m, a0 + 0x34u, 0xFFu);   /* st: collision off */
        wr8(m, a0 + 0x35u, 0u);
        a0 += 0x6Cu;
        d0w = (int16_t)(d0w - 1);
    } while (d0w >= 0);
}

/* ======================================================================== */
/* ROM 0x0C6CE6: arm the respawn tick and copy the life counters.            */
/* ======================================================================== */
void Player_ArmRespawn(const f3mem_t *m, uint32_t a0)
{
    wr32(m, a0 + PS_TICK_FN, PLAYER_FN_RESPAWN);
    /* source is selected by ID; in every call site of this lane the current
     * ID is 0, so the long cond-load degenerates to bit 0 of *(0x1D3A). */
    wr16(m, a0 + PS_SHOTS_LEFT, rd16(m, G_LIVES_P1));
    wr16(m, a0 + PS_AUX_60, rd16(m, G_LIVES_P2));
    wr16(m, a0 + PS_SHIELD_TMR, 0u);
    wr16(m, a0 + PS_AUX_52, 0u);
    wr16(m, a0 + PS_CHILD_PTR, 0u);
    wr8(m, a0 + PS_AUX_64, (uint8_t)(rd8(m, a0 + PS_AUX_64) & (uint8_t)~0x01u));
}

/* ======================================================================== */
/* ROM 0x0C6D0E: respawn: shot object, position, HP, invulnerability, the    */
/* power/multiplier penalties, then 0x0C8E1A + 0x0C8BCE (both not done).     */
/* ======================================================================== */
int Player_Respawn(const f3mem_t *m, uint32_t a0)
{
    /* Obj_Spawn(*(A0+0x62), fn 0x0C95DE, group 1, n 11, arg *(A0+0x62)) */
    Obj_Spawn(m, rd32(m, a0 + PS_CHILD_PTR), TBL_SHOT_SCRIPT, 1u, 11u,
              rd32(m, a0 + PS_CHILD_PTR));
    wr8(m, a0 + PS_FLAGS, 0u);

    /* lea (0x253A,PC),A1 + mulu.w #4 of the id: spawn position pair */
    {
        uint32_t a1 = TBL_SPAWN_XY + 4u * (uint32_t)(uint16_t)rd16(m, a0 + PS_PLAYER_ID);
        wr16(m, a0 + PS_X, rd16(m, a1));
        a1 += 2u;
        wr16(m, a0 + PS_Y, rd16(m, a1));
    }
    wr32(m, a0 + PS_HP, 0x00010000u);
    wr32(m, a0 + PS_DMG, 0x00010000u);
    wr16(m, a0 + PS_INVULN, 0x00D2u);

    /* divu.w #3: take 3 ticks off the multiplier, then its remainder */
    {
        uint16_t v = rd16(m, a0 + PS_MULT_SHOT);
        uint32_t d0 = ((uint32_t)v << 16) / 3u;
        if ((uint16_t)d0 != 0u)                     /* tst.w D0w / beq */
            wr16(m, a0 + PS_MULT_SHOT,
                 (uint16_t)(rd16(m, a0 + PS_MULT_SHOT) - 3u));
        /* swap D0: remainder into the low word, then sub.w */
        uint32_t sw = (d0 << 16) | (d0 >> 16);
        wr16(m, a0 + PS_MULT_SHOT,
             (uint16_t)(rd16(m, a0 + PS_MULT_SHOT) - (uint16_t)sw));
    }
    /* divu.w #5 (no quotient branch): only the remainder is subtracted */
    {
        uint16_t v = rd16(m, a0 + PS_POWER_B);
        uint32_t d0 = ((uint32_t)v << 16) / 5u;
        uint32_t sw = (d0 << 16) | (d0 >> 16);
        wr16(m, a0 + PS_POWER_B, (uint16_t)(rd16(m, a0 + PS_POWER_B) - (uint16_t)sw));
    }

    wr8(m, a0 + PS_COLL_FLAGS, 0u);
    wr32(m, a0 + PS_BANK, 0u);
    wr16(m, a0 + 0x3Eu, 0u);
    wr16(m, a0 + 0x40u, 0u);
    wr16(m, a0 + PS_SHIELD_HP, 0u);
    wr16(m, a0 + PS_AUX_60, rd16(m, G_LIVES_P2));
    wr16(m, a0 + PS_ARM_PAIR, rd16(m, a0 + PS_POWER_A));
    wr16(m, a0 + PS_ARM_PAIR + 2u, 0xFFFFu);
    wr16(m, a0 + PS_LIFE_PAIR, rd16(m, a0 + PS_POWER_B));
    wr16(m, a0 + PS_LIFE_PAIR + 2u, 0xFFFFu);

    return -1; /* 0x0C8E1A, 0x0C8BCE: not decompiled */
}

/* ======================================================================== */
/* ROM 0x0C6B36: scene init, run once per scene.                            */
/* ======================================================================== */
int Player_SceneInit(const f3mem_t *m)
{
    wr16(m, G_FRAME_COUNTER, 0u);
    wr8(m, G_CTRL_FLAGS, (uint8_t)(rd8(m, G_CTRL_FLAGS) & (uint8_t)~0x1Eu));
    wr8(m, G_BOMB_FLAGS, 0u);
    wr16(m, G_DEMO_LEVEL, 0u);
    wr32(m, G_SHAKE_X, 0u);
    wr32(m, G_SHAKE_Y, 0u);

    /* ROM 0x0C7D9A */
    wr16(m, SHIELD_STATE_ADDR + 0x1Eu, 0u);
    wr16(m, SHIELD_STATE_ADDR + 0x1Cu, 0u);
    wr16(m, G_TIMER_ROW_1, 0u);
    wr16(m, G_TIMER_CNT_1, 0u);
    wr16(m, G_TIMER_ROW_2, 0u);
    wr16(m, G_TIMER_CNT_2, 0u);

    /* 0x0C6B66..0x0C6BD2: jsr 0x001660(TBL_PAL_0C9242, dst, n) x4 */
    Pal_QueueUpload(m, TBL_PAL_0C9242, 0x0354u, 3u);
    Pal_QueueUpload(m, TBL_PAL_0C9242, 0x0360u, 8u);
    Pal_QueueUpload(m, TBL_PAL_0C9242, 0x0094u, 3u);
    Pal_QueueUpload(m, TBL_PAL_0C9242, 0x00A0u, 8u);
    return -1; /* 0x0C6BD6: two-player spawn loop (Player_Respawn stops at 0x0C8E1A) */

#if 0
    /* 0x0C6BD6.. the two-player spawn loop, verbatim register map:
     *   A0 = 0x41B190 (P1); A1 = 0x0CABAC (+0x2684 PC); D0w = *(0x1D38,A5);
     *   D1w = 0; loop {
     *     Obj_Spawn(A0 = *(SP), fn 0x0CABAC, group 2, n 7, arg 0xFFF4)
     *     Obj_Spawn(A0 = *(SP), fn 0x0CABAC, group 2, n 7, arg 0x1B)
     *     *(A0+0x44) = *(A1)+ (io_ch ptr) ; *(A0+0x0C) = *(A1)+ ;
     *     *(A0+0x0D) = *(A1)+ ; *(A0) = 0x0C7480 (stage-fail task) ;
     *     *(A0+0x22) = D1w (player id) ; *(A0+0x004) = 0 ; *(A0+0x034) = 0xFF ;
     *     *(A0+0x01C) = 0 ; *(A0+0x01E) = 0 ; *(A0+0x04C) = -1 (out) ;
     *     *(A0+0x054) = 0 ; *(A0+0x056) = 0 ; *(A0+0x058) = 0 ;
     *     *(A0+0x048) = 0 (score) ; *(A0+0x04E) = 0 ; *(A0+0x042) = 12 ;
     *     *(A0+0x05A) = 0x0F ; *(A0+0x05C) = 12 ; *(A0+0x062) = -1 ;
     *     *(A0+0x069) = 0 ; *(A0+0x068) = 0 ; *(A0+0x066) = -1 then bfclr{0:1};
     *     *(A0+0x07A) = *(A0+0x056) ; *(A0+0x07C) = -1 ;
     *     *(A0+0x06A) = *(A0+0x058) ; *(A0+0x06C) = -1 ;
     *     if (D0w == *(A0+0x22)) { 0x0C6D0E(A0); 0x0C6CE6(A0); }  [player 0]
     *     A0 += 0x8A; D1w++; while (D1w < 2)
     *   } 0x0C6CEC entry: *(A0+0x4C) = *(0x1D3A) / *(A0+0x60) = *(0x1D3C)
     *   0x0C6DEC(); rts
     * The A5+0x1D38 word (the player the scene controls) is not captured in
     * any of the three states used here, so only the step up to 0x001660 is
     * applied. */
#endif
}

/* ======================================================================== */
/* ROM 0x0C6E10: pick one active player of the frame into G_ACTIVE_OBJ.      */
/* ======================================================================== */
void Player_PickActive(const f3mem_t *m)
{
    uint32_t a0 = P1_STATE_ADDR;
    uint32_t a1 = G_ACTIVE_TBL;
    uint32_t d1 = 0u;

    wr32(m, G_ACTIVE_OBJ, 0u);
    for (int16_t d0w = 1; d0w >= 0; d0w--) {
        if ((int16_t)rd16(m, a0 + PS_SHOTS_LEFT) >= 0) {
            wr32(m, a1, a0);
            a1 += 4u;
            d1 = (d1 & 0xFFFF0000u) | ((d1 + 1u) & 0xFFFFu); /* addq.w #1,D1w */
        }
        a0 += 0x8A;
    }
    wr16(m, G_ACTIVE_COUNT, (uint16_t)d1);
    if ((int16_t)d1 == 0)
        return; /* beq: nobody active, 0x1D28 stays 0 */

    /* moveq #0,D0; move.w (0x1456,A5),D0w; divu.w D1w,D0; swap D0:
     * remainder = (0x1456) % count, then move.l (A1,D0w*4),(0x1D28,A5) with
     * D0w sign-extended into the address. */
    uint16_t d0w = (uint16_t)(rd16(m, G_PICK_SEED) % (uint16_t)d1);
    wr32(m, G_ACTIVE_OBJ,
         rd32(m, G_ACTIVE_TBL + 4u * (uint32_t)(int32_t)d0w));
}

/* ======================================================================== */
/* ROM 0x0C77BE / 0x0C77E8: credit-entry checks of the alive-ship tick      */
/* (0x0CFEC, 0x01016, 0x01034, 0x0103A: none decompiled).                    */
/* ======================================================================== */
int Player_CoinCheckA(const f3mem_t *m, uint32_t a0)
{
    if (rd8(m, G_COIN_BUSY) != 0u)
        return 0;
    (void)a0;
    return -1; /* 0x0C77C8: tst.w (0x62,A0); jsr 0x0CFEC / 0x01016 */
}

void Player_CoinCheckB(const f3mem_t *m, uint32_t a0)
{
    if (rd8(m, G_COIN_BUSY) != 0u)
        return;
    if (rd8(m, G_COIN_BUSY_2) != 0u)
        return;
    (void)a0; /* jsr 0x01034 / 0x0103A */
}

/* ======================================================================== */
/* ROM 0x0C7814: per-player stage-fail text (labels TBL_FAIL_TEXT_A/B/C are
 * picked by D0b, then written with 0x571A/0x5566).                          */
/* ======================================================================== */
int Player_FailText(const f3mem_t *m, int32_t d0, uint16_t d2w, uint16_t d3w)
{
    (void)m; (void)d0; (void)d2w; (void)d3w;
    return -1; /* 0x0C7814 body: text writes only */
}

/* ======================================================================== */
/* ROM 0x0C7C60: defeat bookkeeping of the alive-ship tick.                 */
/* ======================================================================== */
int Player_DefeatBookKeeping(const f3mem_t *m, uint32_t a0)
{
    Player_CoinCheckB(m, a0);
    if ((int16_t)rd16(m, a0 + PS_CHILD_PTR) < 0)
        goto LAB_0C7CBE;

    /* jsr 0x13D638 (hi-score slot lookup, not decompiled) then 0x0C7C94,
     * which only feeds D1w:
     *   bfset (0x66,A0){0:1}; clr.l (0x48,A0); Obj_Spawn(A0, 0x11A956, g2..7, 0)
     * and rejoins at LAB_0C7CC2. Which of the two branches runs depends on
     * D1w from 0x13D638, so both effects are left out until it exists. */
    return -1;

LAB_0C7CBE:
    wr32(m, a0 + PS_SCORE, 0u);
    /* LAB_0C7CC2 */
    if (Player_Respawn(m, a0) != 0)
        return -1;
    Player_ArmRespawn(m, a0);
    return 0;
}

/* ======================================================================== */
/* ROM 0x0C84E4: queue a 25-entry HUD palette upload and shift the ramp.    */
/* ======================================================================== */
static void hud_row1_player(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t rem)
{
    uint16_t id = rd16(m, a0 + PS_PLAYER_ID);
    uint16_t pal = (uint16_t)rd8(m, 0x000C8548u + id);
    uint16_t dst = (uint16_t)((pal << 6) + 0x4004u);
    Pal_QueueUpload(m, a1, dst, 0x0019u);
    for (int i = 0; i < 24; i++) {
        wr32(m, a1, rd32(m, a1 + 4u));
        a1 += 4u;
    }
    if (rd16(m, a0 + PS_SHIELD_TMR) != 0u) {
        wr32(m, a1, 0x00FF2020u);
    } else {
        uint16_t idx = (uint16_t)(rd16(m, a0 + PS_POWER_B) / 5u);
        uint32_t tbl = rd32(m, 0x000C854Au + 4u * (uint32_t)idx);
        wr32(m, a1, rd32(m, tbl + 4u * (uint32_t)rem));
    }
}

/* ======================================================================== */
/* ROM 0x0C8488: HUD animation-row 1 advance + 0x001660 palette uploads.    */
/* ======================================================================== */
int Player_HudRow1(const f3mem_t *m)
{
    wr16(m, G_TIMER_CNT_1, (uint16_t)(rd16(m, G_TIMER_CNT_1) + 1u));
    uint16_t d1 = rd16(m, G_TIMER_ROW_1);
    uint16_t d0 = rd16(m, G_TIMER_CNT_1);
    if ((int16_t)d0 >= (int16_t)rd16(m, 0x000C8746u + d1)) {
        wr16(m, G_TIMER_CNT_1, 0u);
        d1 = (uint16_t)(d1 + 6u);
        if ((int16_t)rd16(m, 0x000C8746u + d1) < 0)
            d1 = 0u;
    }
    wr16(m, G_TIMER_ROW_1, d1);
    uint16_t rem = (uint16_t)(rd16(m, G_FRAME_COUNTER) % 0x19u);
    hud_row1_player(m, P1_STATE_ADDR, TEXT_STATE_P1, rem);
    hud_row1_player(m, P2_STATE_ADDR, TEXT_STATE_P2, rem);
    return 0;
}

/* ======================================================================== */
/* ROM 0x0C87F8: HUD animation-row 2 advance.                               */
/* ======================================================================== */
void Player_HudRow2(const f3mem_t *m)
{
    wr16(m, G_TIMER_CNT_2, (uint16_t)(rd16(m, G_TIMER_CNT_2) + 1u));
    uint16_t d1 = rd16(m, G_TIMER_ROW_2);
    uint16_t d0 = rd16(m, G_TIMER_CNT_2);
    if ((int16_t)d0 >= (int16_t)rd16(m, 0x000C8882u + d1)) {
        wr16(m, G_TIMER_CNT_2, 0u);
        d1 = (uint16_t)(d1 + 6u);
        if ((int16_t)rd16(m, 0x000C8882u + d1) < 0)
            d1 = 0u;
    }
    wr16(m, G_TIMER_ROW_2, d1);
}



/* ---- sound helper: ROM 0x13F238 remap + ROM 0x0039D8 Sound_PlayCmd ---- */
static void play_sfx_remapped(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    Sound_PlayCmd(m, cmd);
}

/* ======================================================================== */
/* ROM 0x0B90B6 / 0x0B9080: object-script / terrain-collision pass.         */
/* ======================================================================== */
static void player_script_one(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    if (rd32(m, a0 + PS_IO_CH) != a1) {
        wr32(m, a0 + PS_IO_CH, a1);
        if (rd8(m, F3_A5 + 0x08A0u) & 0x80u) {
            wr16(m, a0 + PS_MULT_SHOT, 9u);
            wr16(m, a0 + PS_POWER_A, 0x21u);
            wr16(m, a0 + PS_ARM_PAIR, 0x20u);
            wr16(m, a0 + PS_POWER_B, 0x0Eu);
            wr16(m, a0 + PS_LIFE_PAIR, 0x0Du);
            wr16(m, a0 + PS_SHIELD_HP, 0x64u);
        }
    }

    uint8_t d7 = 0xFFu;
    int16_t x = (int16_t)((int16_t)rd16(m, a0 + PS_X) + 0x14);
    int16_t y = (int16_t)((int16_t)rd16(m, a0 + PS_Y) + 0x0E);
    int16_t d2 = 0;
    int found = 0;
    do {
        x = (int16_t)(x + 8);
        if ((uint8_t)Terrain_TestPoint(m, x, y) != 0u) {
            found = 1;
            break;
        }
        d2++;
    } while (d2 < 3);

    if (found) {
        if (d2 <= 1)
            d7 &= (uint8_t)~(1u << 2);
        int16_t y_up = y;
        for (;;) {
            y = (int16_t)(y + 8);
            if (y >= 0xC2) {
                d7 &= (uint8_t)~(1u << 0);
                break;
            }
            if ((uint8_t)Terrain_TestPoint(m, x, y) == 0u) {
                d7 &= (uint8_t)~(1u << 1);
                break;
            }
            y_up = (int16_t)(y_up - 8);
            if (y_up <= 0x1E) {
                d7 &= (uint8_t)~(1u << 1);
                break;
            }
            if ((uint8_t)Terrain_TestPoint(m, x, y_up) == 0u) {
                d7 &= (uint8_t)~(1u << 0);
                break;
            }
        }
    } else {
        uint32_t a2 = rd32(m, 0x0041A988u + 4u * (uint32_t)rd16(m, a0 + PS_PLAYER_ID));
        int16_t cx = (int16_t)rd16(m, a2 + 0x14u);
        if (cx < 0x104 && cx > 0x28) {
            int16_t lo = (int16_t)(cx - 0x96);
            int16_t hi = (int16_t)(cx - 0x78);
            int16_t px = (int16_t)rd16(m, a0 + PS_X);
            if (px < lo) d7 &= (uint8_t)~(1u << 3);
            if (px > hi) d7 &= (uint8_t)~(1u << 2);
        }
        int16_t cy = (int16_t)rd16(m, a2 + 0x18u);
        if (cy < 0xC0 && cy > 0x14) {
            int16_t lo = (int16_t)(cy - 0x0F);
            int16_t hi = (int16_t)(cy + 0x0F);
            int16_t py = (int16_t)rd16(m, a0 + PS_Y);
            if (py < lo) d7 &= (uint8_t)~(1u << 1);
            if (py > hi) d7 &= (uint8_t)~(1u << 0);
        }
    }

    x = (int16_t)((int16_t)rd16(m, a0 + PS_X) + 0x14);
    y = (int16_t)((int16_t)rd16(m, a0 + PS_Y) + 0x0E);
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(x + 0x0A), y) != 0u) d7 |= (uint8_t)(1u << 3);
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(x - 0x0A), y) != 0u) d7 |= (uint8_t)(1u << 2);
    if ((uint8_t)Terrain_TestPoint(m, x, (int16_t)(y + 0x0A)) != 0u) d7 |= (uint8_t)(1u << 1);
    if ((uint8_t)Terrain_TestPoint(m, x, (int16_t)(y - 0x0A)) != 0u) d7 |= (uint8_t)(1u << 0);

    uint8_t d0 = d7;
    if ((rd16(m, G_FRAME_COUNTER) & 3u) == 0u)
        d0 &= (uint8_t)~(1u << 4);
    if ((Rng_NextMem(m) & 0x3FFu) == 0u)
        d0 &= (uint8_t)~(1u << 5);

    uint8_t d1 = rd8(m, a1);
    wr8(m, a1 + 2u, d1);
    wr8(m, a1, d0);
    wr8(m, a1 + 1u, (uint8_t)(d0 | (uint8_t)~d1));
    wr8(m, a1 + 3u, (uint8_t)((uint8_t)~d0 | d1));

    if (rd8(m, G_PAUSE) & 0x04u) {
        uint8_t bf = (uint8_t)((rd8(m, F3_A5 + 0x089Cu) >> 4) & 3u);
        uint8_t bit;
        if (bf == 2u) bit = 0u;
        else if (bf == 1u) bit = 1u;
        else bit = (uint8_t)(Rng_NextMem(m) & 3u);
        wr8(m, a1 + 1u, (uint8_t)(rd8(m, a1 + 1u) & (uint8_t)~((1u << bit) | (1u << 4))));
    }
    if ((int16_t)rd16(m, a0 + PS_SHOTS_LEFT) < 0 && !(rd8(m, F3_A5 + 0x08A0u) & 0x80u))
        wr8(m, a1 + 1u, (uint8_t)(rd8(m, a1 + 1u) & (uint8_t)~0x80u));
}

static void player_script_pass(const f3mem_t *m)
{
    uint32_t a0 = P1_STATE_ADDR;
    uint32_t a1 = F3_A5 + 0x08A1u;
    for (int i = 0; i < 2; i++) {
        uint16_t d1 = rd16(m, a0 + PS_PLAYER_ID);
        if (rd8(m, F3_A5 + 0x08A0u) & (1u << (d1 & 7u)))
            player_script_one(m, a0, a1);
        a0 += 0x8Au;
        a1 += 4u;
    }
}

/* ======================================================================== */
/* ROM 0x0C8460: draw object sprite with screen-shake offset.               */
/* ======================================================================== */
static void player_draw_shaken(const f3mem_t *m, uint32_t a0)
{
    uint16_t ox = rd16(m, a0 + PS_X);
    uint16_t oy = rd16(m, a0 + PS_Y);
    wr16(m, a0 + PS_X, (uint16_t)(ox + rd16(m, G_SHAKE_X)));
    wr16(m, a0 + PS_Y, (uint16_t)(oy + rd16(m, G_SHAKE_Y)));
    Obj_DrawSprite(m, a0);
    wr16(m, a0 + PS_Y, oy);
    wr16(m, a0 + PS_X, ox);
}

/* ======================================================================== */
/* ROM 0x0C7CD0: clamp player score and draw score digits to text RAM.      */
/* ======================================================================== */
static void player_score_hud(const f3mem_t *m, uint32_t a0)
{
    int32_t score = (int32_t)rd32(m, a0 + PS_SCORE);
    if (score < 0)
        score = 0;
    else if (score > 0x05F5E0FF)
        score = 0x05F5E0FF;
    wr32(m, a0 + PS_SCORE, (uint32_t)score);
    if (m->gfx) {
        uint16_t id = rd16(m, a0 + PS_PLAYER_ID);
        uint32_t dst = (id == 0u) ? 0x0061C000u : 0x0061C02Cu;
        /* 0x005666 writes the label, 0x00571A writes the 8-digit score */
        Gfx_DrawDecimal(m, (uint32_t)score, 8u, dst + 2u * 8u, 7u);
    }
}

/* ======================================================================== */
/* ROM 0x0C98B4: fire primary weapon shots when fire counter ready.         */
/* ======================================================================== */
static void player_fire_primary(const f3mem_t *m, uint32_t a0)
{
    uint16_t mult = rd16(m, a0 + PS_MULT_SHOT);
    uint32_t a2 = rd32(m, 0x000CA55Cu + 4u * (uint32_t)mult);
    int16_t cnt = (int16_t)rd16(m, a2);
    a2 += 2u;
    if (cnt > (int16_t)rd16(m, a0 + PS_FIRE_COUNT))
        return;
    wr16(m, a0 + 0x3Eu, rd16(m, a2));
    a2 += 2u;
    wr16(m, a0 + 0x40u, 0u);
    uint16_t id = rd16(m, a0 + PS_PLAYER_ID);
    play_sfx_remapped(m, (int16_t)rd16(m, a2 + 2u * (uint32_t)id));
    a2 += 4u;
    for (int16_t i = (int16_t)(cnt - 1); i >= 0; i--) {
        uint32_t fn = rd32(m, a2);
        a2 += 4u;
        Obj_Spawn(m, a0, fn, 7u, 2u, a2);
        a2 += 6u;
    }
}

/* ======================================================================== */
/* ROM 0x0C9916: fire secondary weapon shots when weapon counter ready.     */
/* ======================================================================== */
static void player_fire_secondary(const f3mem_t *m, uint32_t a0)
{
    int16_t pa = (int16_t)rd16(m, a0 + PS_POWER_A);
    int16_t idx = (int16_t)(pa / 3);
    uint32_t a2 = rd32(m, 0x000CA778u + 4u * (uint32_t)(int32_t)idx);
    int16_t thresh = (int16_t)rd16(m, a2);
    a2 += 2u;
    if (thresh > (int16_t)rd16(m, a0 + PS_WEAPON_A))
        return;
    int16_t cnt = (int16_t)rd16(m, a2);
    a2 += 2u;
    uint16_t id = rd16(m, a0 + PS_PLAYER_ID);
    play_sfx_remapped(m, (int16_t)rd16(m, a2 + 2u * (uint32_t)id));
    a2 += 4u;
    for (int16_t i = (int16_t)(cnt - 1); i >= 0; i--) {
        uint32_t fn = rd32(m, a2);
        a2 += 4u;
        Obj_Spawn(m, a0, fn, 7u, 2u, a2);
        a2 += 4u;
    }
}

/* ======================================================================== */
/* ROM 0x0C8828: draw ship thruster flame.                                  */
/* ======================================================================== */
static void player_draw_thruster(const f3mem_t *m, uint32_t a0)
{
    if (rd8(m, a0 + PS_FLAGS) & 0x04u)
        return;
    if (rd16(m, G_DEMO_LEVEL) != 0u)
        return;
    uint32_t sh = SHIELD_STATE_ADDR;
    wr16(m, sh + 0x1Eu, 0u);
    wr16(m, sh + 0x1Cu, 0u);
    wr16(m, sh + PS_X, (uint16_t)(rd16(m, a0 + PS_X) - 8u));
    wr16(m, sh + PS_Y, (uint16_t)(rd16(m, a0 + PS_Y) + 7u));
    uint16_t row = rd16(m, G_TIMER_ROW_2);
    wr32(m, sh + 0x10u, rd32(m, 0x000C8882u + 2u + (uint32_t)row));
    wr8(m, sh + 0x1Du, 0u);
    player_draw_shaken(m, sh);
}

/* ======================================================================== */
/* ROM 0x0C889C: draw muzzle flash sequence.                                */
/* ======================================================================== */
static void player_draw_muzzle(const f3mem_t *m, uint32_t a0)
{
    if (rd8(m, a0 + PS_FLAGS) & 0x04u)
        return;
    uint32_t sh = SHIELD_STATE_ADDR;
    wr8(m, sh + 0x1Du, 0u);
    uint32_t a2 = 0x000C8914u + rd16(m, a0 + 0x3Eu);
    if ((int16_t)rd16(m, a2) <= 0)
        return;
    int16_t d0 = (int16_t)rd16(m, a0 + 0x40u);
    if (d0 <= 0) {
        d0 = (int16_t)rd16(m, a2);
        a2 += 2u;
        wr16(m, a0 + 0x40u, (uint16_t)d0);
        wr16(m, a0 + 0x3Eu, (uint16_t)(rd16(m, a0 + 0x3Eu) + 2u));
    }
    for (int16_t i = (int16_t)(d0 - 1); i >= 0; i--) {
        int8_t dx = (int8_t)rd8(m, a2);
        int8_t dy = (int8_t)rd8(m, a2 + 1u);
        uint32_t spr = rd32(m, a2 + 2u);
        a2 += 6u;
        wr16(m, sh + PS_X, (uint16_t)((int16_t)rd16(m, a0 + PS_X) + (int16_t)dx));
        wr16(m, sh + PS_Y, (uint16_t)((int16_t)rd16(m, a0 + PS_Y) + (int16_t)dy));
        wr32(m, sh + 0x10u, spr);
        player_draw_shaken(m, sh);
        wr16(m, a0 + 0x3Eu, (uint16_t)(rd16(m, a0 + 0x3Eu) + 6u));
    }
}

/* ======================================================================== */
/* ROM 0x0C7946: alive-ship tick.                                           */
/* ======================================================================== */
int Player_AliveShipTick(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    wr8(m, a0 + 0x68u, rd8(m, F3_A5 + 0x32DFu));
    wr8(m, a0 + 0x66u, (uint8_t)(rd8(m, a0 + 0x66u) & 0x7Fu));
    player_score_hud(m, a0);

    int16_t sh_hp = (int16_t)rd16(m, a0 + PS_SHIELD_HP);
    if (sh_hp > 2) sh_hp = 2;
    uint32_t a2 = 0x000C9482u + (uint32_t)((int32_t)sh_hp << 3);
    wr32(m, a0 + 0x2Cu, rd32(m, a2));
    wr32(m, a0 + 0x30u, rd32(m, a2 + 4u));

    if (!(rd8(m, G_CTRL_FLAGS) & 0x04u) && !(rd8(m, a0 + PS_FLAGS) & 0x02u)) {
        if (!(rd8(m, a1 + 1u) & 0x20u) && (int16_t)rd16(m, a0 + PS_AUX_60) > 0) {
            uint8_t bf = rd8(m, G_BOMB_FLAGS);
            wr8(m, G_BOMB_FLAGS, (uint8_t)(bf | 0x01u));
            if (!(bf & 0x01u)) {
                wr8(m, G_BOMB_FLAGS, (uint8_t)(rd8(m, G_BOMB_FLAGS) & 0x7Fu));
                Obj_Spawn(m, a0, 0x000CAEBAu, 1u, 2u, a1);
                Obj_Spawn(m, a0, 0x000CB97Eu, 1u, 2u, 0x000C915Au);
            }
        }
        if (!(rd8(m, a1 + 1u) & 0x10u)) {
            wr16(m, a0 + PS_AUX_52, 0x0100u);
            wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x30u);
        }
        if (!(rd8(m, a1) & 0x10u)) {
            wr16(m, a0 + PS_AUX_52,
                 (uint16_t)(rd16(m, a0 + PS_AUX_52) + rd16(m, G_DEMO_MODE)));
        }
    }

    if (rd8(m, a0 + PS_AUX_52) != 0u) {
        wr8(m, a0 + PS_AUX_52, 0u);
        player_fire_secondary(m, a0);
        player_fire_primary(m, a0);
    }

    player_draw_thruster(m, a0);
    player_draw_muzzle(m, a0);

    int16_t dlvl = (int16_t)rd16(m, G_DEMO_LEVEL);
    if (dlvl > 4) {
        wr32(m, a0 + 0x10u, (dlvl > 9) ? 0x0002A3D0u : 0x0002A3B4u);
    } else {
        int16_t bank = (int16_t)rd16(m, a0 + PS_BANK);
        wr32(m, a0 + 0x10u, rd32(m, 0x000C927Eu + (uint32_t)((int32_t)bank * 4)));
    }

    if ((int16_t)rd16(m, a0 + PS_INVULN) > 0) {
        wr16(m, a0 + PS_INVULN, (uint16_t)(rd16(m, a0 + PS_INVULN) - 1u));
        if ((int16_t)rd16(m, a0 + PS_INVULN) >= 0x96)
            wr32(m, a0 + PS_POS_X, rd32(m, a0 + PS_POS_X) + 0x00018000u);
        if (!(rd8(m, a0 + PS_FLAGS) & 0x04u)) {
            if (!(rd8(m, a0 + PS_INVULN + 1u) & 0x04u))
                wr8(m, a0 + 0x1Du, 8u);
            player_draw_shaken(m, a0);
            uint16_t sx = rd16(m, a0 + PS_X);
            uint16_t sy = rd16(m, a0 + PS_Y);
            wr8(m, a0 + 0x1Du, rd8(m, a0 + PS_PAL_BANK));
            uint16_t id = rd16(m, a0 + PS_PLAYER_ID);
            uint32_t tbl = 0x000C94A2u + 8u * (uint32_t)id;
            wr16(m, a0 + PS_X, (uint16_t)(sx + rd16(m, tbl)));
            wr16(m, a0 + PS_Y, (uint16_t)(sy + rd16(m, tbl + 2u)));
            wr32(m, a0 + 0x10u, rd32(m, tbl + 4u));
            player_draw_shaken(m, a0);
            wr16(m, a0 + PS_Y, sy);
            wr16(m, a0 + PS_X, sx);
        }
        wr32(m, a0 + PS_HP, 0x00010000u);
        wr32(m, a0 + PS_TICK_FN, PLAYER_FN_SHIP);
        return 0;
    }

    if (!(rd8(m, a0 + PS_FLAGS) & 0x04u))
        player_draw_shaken(m, a0);
    wr8(m, a0 + 0x1Du, rd8(m, a0 + PS_PAL_BANK));

    if ((int32_t)rd32(m, a0 + PS_HP) <= 0) {
        if ((int16_t)rd16(m, a0 + PS_SHIELD_HP) > 0) {
            wr32(m, a0 + PS_HP, 0x00010000u);
            if ((int16_t)rd16(m, a0 + PS_SHIELD_TMR) <= 0) {
                wr16(m, a0 + PS_SHIELD_TMR, 0x000Au);
                play_sfx_remapped(m, 0x0E);
            }
            wr32(m, a0 + PS_TICK_FN, PLAYER_FN_SHIP);
            return 0;
        }
        wr8(m, a0 + PS_FLAGS, (uint8_t)(rd8(m, a0 + PS_FLAGS) | 0x01u));
        uint16_t id = rd16(m, a0 + PS_PLAYER_ID);
        play_sfx_remapped(m, (int16_t)rd16(m, 0x000C92CEu + 2u * (uint32_t)id));
        uint16_t pal = (uint16_t)(((uint16_t)rd8(m, a0 + PS_PAL_BANK) << 6) + 0x4000u);
        Pal_QueueUpload(m, rd32(m, 0x000C92D2u + 4u * (uint32_t)id), pal, 0x0020u);
        wr8(m, a0 + PS_COLL_FLAGS, 0xFFu);
        wr16(m, a0 + PS_BANK, 0u);
        wr16(m, a0 + 0x3Eu, 0u);
        wr16(m, a0 + PS_X, (uint16_t)(rd16(m, a0 + PS_X) - 0x20u));
        wr16(m, a0 + PS_Y, (uint16_t)(rd16(m, a0 + PS_Y) - 0x11u));
        wr32(m, a0 + PS_TICK_FN, 0x000C7B48u);
        return 0;
    }

    wr32(m, a0 + PS_TICK_FN, PLAYER_FN_SHIP);
    return 0;
}

/* ======================================================================== */
/* ROM 0x0C8E52, 0x0C8CCE, 0x0C8BFA, 0x0C8EE8, 0x0C8AB4, 0x0C8B8A, 0x0C8F48 */
/* and 0x0C8A5E (Player_Hud).                                               */
/* ======================================================================== */
static uint32_t hud_text_addr(const f3mem_t *m, uint32_t a1, uint32_t a2)
{
    uint16_t id = rd16(m, a1 + PS_PLAYER_ID);
    uint16_t x = (uint16_t)(rd16(m, 0x000C914Au + 4u * (uint32_t)id) + rd16(m, a2 + 4u));
    uint16_t y = rd16(m, 0x000C914Au + 4u * (uint32_t)id + 2u);
    return 0x0061C000u + (uint32_t)((x << 1) + (y << 7));
}

static void hud_draw_bar_step(const f3mem_t *m, uint32_t a1, uint32_t a2,
                              uint16_t rem, uint16_t quot)
{
    uint16_t id = rd16(m, a1 + PS_PLAYER_ID);
    if (m->gfx) {
        uint32_t tbl = rd32(m, a2 + 0x0Au);
        uint32_t src = rd32(m, tbl + 4u * (uint32_t)id + 8u * (uint32_t)quot);
        Gfx_DrawTilemapBlock(m, src, hud_text_addr(m, a1, a2), 0u);
    }
    uint16_t dst = (uint16_t)(rd16(m, 0x000C915Eu + 4u * (uint32_t)id + 2u) +
                              (rd16(m, a2 + 2u) << 2));
    if (rem != 0u)
        Pal_QueueUpload(m, rd32(m, a2 + 0x0Eu), dst, rem);
    uint16_t left = (uint16_t)(rd16(m, a2) - rem);
    Pal_QueueUpload(m, TBL_PAL_0C9242, (uint16_t)(dst + (rem << 2)), left);
}

static uint8_t hud_levelup_anim(const f3mem_t *m, uint32_t a1, uint32_t a2,
                                uint32_t a3, uint16_t rem, uint16_t quot)
{
    for (;;) {
        uint16_t off = rd16(m, a3 + 2u);
        int16_t dur = (int16_t)rd16(m, 0x000C8FB2u + (uint32_t)off);
        if (dur <= 0)
            return 0xFFu;
        int16_t cnt = (int16_t)rd16(m, a3 + 4u);
        if (cnt >= dur) {
            wr16(m, a3 + 4u, 0u);
            wr16(m, a3 + 2u, (uint16_t)(off + 6u));
            continue;
        }
        wr16(m, a3 + 4u, (uint16_t)(cnt + 1));
        if (cnt > 0)
            return 0u;
        uint32_t vram = m->gfx ? hud_text_addr(m, a1, a2) : 0u;
        uint32_t arg = rd32(m, 0x000C8FB2u + (uint32_t)off + 2u);
        if ((uint16_t)arg != 0u) {
            if (m->gfx)
                Gfx_DrawTilemapBlock(m, arg, vram, 0u);
        } else if (rem <= 1u) {
            if (m->gfx) {
                uint32_t tbl = rd32(m, a2 + 0x12u);
                uint16_t id = rd16(m, a1 + PS_PLAYER_ID);
                uint32_t tsrc = rd32(m, tbl + 4u * (uint32_t)id);
                Gfx_DrawTilemapBlock(m, tsrc, vram, 0u);
                uint16_t slot = rd8(m, tsrc + 5u);
                uint32_t ctbl = rd32(m, a2 + 6u);
                uint32_t csrc = rd32(m, ctbl + 4u * (uint32_t)((quot << 1) + rem));
                Gfx_UploadTextTiles(m, csrc, slot);
            }
        } else {
            if (rem == 3u) {
                uint16_t id = rd16(m, a1 + PS_PLAYER_ID);
                uint16_t dst = (uint16_t)(rd16(m, 0x000C915Eu + 4u * (uint32_t)id + 2u) +
                                          (rd16(m, a2 + 2u) << 2));
                Pal_QueueUpload(m, TBL_PAL_0C9242, dst, rd16(m, a2));
            } else if (rem > 3u) {
                uint16_t nxt = (uint16_t)(rd16(m, a3) + 1u);
                uint16_t step = rd16(m, a2);
                hud_draw_bar_step(m, a1, a2, (uint16_t)(nxt % step), (uint16_t)(nxt / step));
            }
            if (m->gfx) {
                uint16_t idx = (uint16_t)(rem - 2u);
                uint32_t tbl = rd32(m, a2 + 0x16u);
                vram += (uint32_t)(int16_t)rd16(m, 0x000C903Cu + 2u * (uint32_t)idx);
                Gfx_DrawTilemapBlock(m, rd32(m, tbl + 4u * (uint32_t)idx), vram, 0u);
            }
        }
        return 0u;
    }
}

static void hud_bar_update(const f3mem_t *m, uint32_t a1, uint32_t a2, uint32_t a3, uint16_t target)
{
    if ((int16_t)rd16(m, a3 + 2u) < 0) {
        if ((int16_t)target <= (int16_t)rd16(m, a3))
            return;
        wr16(m, a3 + 2u, 0u);
        wr16(m, a3 + 4u, 0xFFF7u);
        wr8(m, a3 + 6u, 0u);
    }
    uint16_t step = rd16(m, a2);
    uint16_t cur = rd16(m, a3);
    uint16_t quot = (uint16_t)(cur / step);
    uint16_t rem = (uint16_t)(cur % step);
    if ((int16_t)rd16(m, a3 + 4u) < 0) {
        int16_t idx = (int16_t)rd16(m, a3 + 4u);
        wr16(m, a3 + 4u, (uint16_t)(idx + 1));
        uint16_t id = rd16(m, a1 + PS_PLAYER_ID);
        uint16_t dst = (uint16_t)(rd16(m, 0x000C915Eu + 4u * (uint32_t)id + 2u) +
                                  ((rd16(m, a2 + 2u) + rem) << 2));
        uint32_t src = (uint32_t)(0x000C8CCEu + (int32_t)idx * 4);
        Pal_QueueUpload(m, src, dst, 1u);
        return;
    }
    if ((uint16_t)(rem + 1u) >= step) {
        if (hud_levelup_anim(m, a1, a2, a3, rem, quot) == 0u)
            return;
    }
    cur = (uint16_t)(cur + 1u);
    wr16(m, a3, cur);
    hud_draw_bar_step(m, a1, a2, (uint16_t)(cur % step), (uint16_t)(cur / step));
    wr16(m, a3 + 2u, 0xFFFFu);
}

static void hud_bar_pulse(const f3mem_t *m, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint16_t quot = (uint16_t)(rd16(m, a3) / rd16(m, a2));
    uint32_t base = rd32(m, 0x000C91A6u + 4u * (uint32_t)quot);
    wr8(m, a3 + 7u, (uint8_t)(rd8(m, a3 + 7u) + 3u));
    uint16_t f = (uint16_t)((rd8(m, a3 + 7u) >> 4) & 0x0Fu);
    if (f >= 6u) {
        f = 0u;
        wr8(m, a3 + 7u, (uint8_t)(rd8(m, a3 + 7u) & 0x0Fu));
    }
    uint32_t src = (uint32_t)((int32_t)base + 0x14 - (int32_t)f);
    uint16_t id = rd16(m, a1 + PS_PLAYER_ID);
    uint16_t dst = (uint16_t)(rd16(m, 0x000C915Eu + 4u * (uint32_t)id) + 0x10u);
    Pal_QueueUpload(m, src, dst, 6u);
}

static void hud_global_pulse(const f3mem_t *m)
{
    uint16_t cnt = rd16(m, G_FRAME_COUNTER);
    if ((cnt & 7u) != 0u)
        return;
    int32_t idx = -(int32_t)((cnt >> 3) & 3u);
    uint32_t src = (uint32_t)(0x000C9196u + idx * 4);
    Pal_QueueUpload(m, src, 0x0418u, 4u);
    Pal_QueueUpload(m, src, 0x04D8u, 4u);
    Pal_QueueUpload(m, src, 0x0518u, 4u);
}

int Player_Hud(const f3mem_t *m)
{
    uint32_t a1 = P1_STATE_ADDR;
    for (int i = 0; i < 2; i++) {
        if ((int16_t)rd16(m, a1 + PS_SHOTS_LEFT) >= 0) {
            hud_bar_update(m, a1, 0x000C9008u, a1 + PS_ARM_PAIR, rd16(m, a1 + PS_POWER_A));
            hud_bar_update(m, a1, 0x000C9022u, a1 + PS_LIFE_PAIR, rd16(m, a1 + PS_POWER_B));
            hud_bar_pulse(m, a1, 0x000C9022u, a1 + PS_LIFE_PAIR);
            /* 0x0C8AB4 (lives) and 0x0C8B8A (bombs) only write to 0x61C000 text RAM. */
        }
        a1 += 0x8Au;
    }
    hud_global_pulse(m);
    return 0;
}

/* ======================================================================== */
/* ROM 0x0C6E5A: stage-fail sequence. It never returns to the caller and its */
/* frames are TRAP #5 yields, so the frame loop models it by `continue`; it   */
/* stops here because the text writers are not decompiled.                   */
/* ======================================================================== */
int Player_StageFail(const f3mem_t *m, uint32_t p)
{
    (void)m; (void)p;
    return -1; /* 0x0C6E6E on: 0x5566 x13, 0x0C77BE/0x0C7814, 0x0C7C60, 0x978 */
}

/* ======================================================================== */
/* ROM 0x0C6A64 Task_PlayerControl.                                         */
/*                                                                          */
/* Re-entry. The ROM is called from its scheduler slot, but the task itself */
/* yields (TRAP #5) and after every resume restarts at 0x0C6A64 itself (the  */
/* PC of 0x0C6A9A is 0x0C6A64 + 0x36, i.e. the saved return address of the   */
/* *slot call*, so each resumed yield re-runs the SPAWN and 0x0C6B36). The   */
/* measured effect is a frame-slotted state machine (02, MAME 0.289):        */
/*   frame k+1 executes the instruction right after the instruction that was */
/*   waiting at 0x0C6A9A... actually: 0x0C6A9A -> resume at 0x0C6A9C,        */
/*   0x0C6A9C's trap -> 0x0C6AA0, 0x0C6AB0.. -> 0x0C6B16, then the slot     */
/*   restarts at 0x0C6A64 with the loop re-entered at 0x0C6A9A.            */
/* A non-persistent MAME breakpoint shifts every later stop one instruction  */
/* earlier (measured: k stops at, k+1 skips the first), so the pre/post pairs */
/* of ANY capture function of this task are only comparable per frame after  */
/* the previous execution finished, never per call.                          */
/*                                                                          */
/* The model below therefore takes the task as a permanent loop (what the    */
/* kernel sees) and, by default, runs it only up to the first yield so that  */
/* every side effect applied is one of the first frame. per_frame > 0 modes  */
/* documented behaviours for the yield/resume slot order.                    */
/* ======================================================================== */
int PlayerControl_RunFrame(const f3mem_t *m)
{
    /* 0C6A64 bset.b #0,(0x1D1E,A5): Z = previous bit */
    uint8_t old = rd8(m, G_SCENE_STARTED);
    wr8(m, G_SCENE_STARTED, (uint8_t)(old | 0x01u));
    if (!(old & 0x01u)) {
        wr16(m, G_DEMO_MODE, 0x001Du);   /* 0C6A6E */
        wr8(m, G_CTRL_FLAGS, 0u);        /* 0C6A74 */
        /* 0C6A7A: Obj_Spawn(A0 = 0, fn 0x0C685A, group 2, n 7, arg 0) */
        Obj_Spawn(m, 0u, 0x000C685Au, 2u, 7u, 0u);
        if (Player_SceneInit(m) != 0)              /* 0C6A96 */
            return -1;
    }

    /* LAB_0C6A9A: TRAP #5 resumes at LAB_0C6A9C */
    wr16(m, G_FRAME_COUNTER, (uint16_t)(rd16(m, G_FRAME_COUNTER) + 1u));
    if (rd8(m, G_PAUSE) & 0x02u)
        return 0;                        /* paused: the whole frame is the yield */

    /* 0C6AA8: move.l #0x41a988,(0x1d4c,A5) -- COLL_LIST1 head */
    wr32(m, G_ACTIVE_LIST, 0x0041A988u);

    /* 0C6AB0: jsr 0x0B9080 -- object-script / terrain-collision pass */
    player_script_pass(m);

    /* 0C6AB6..0C6B32 */
    if (Player_HudRow1(m) != 0) return -1;
    Player_HudRow2(m);
    {
        uint32_t a0 = P1_STATE_ADDR;
        int rc = 0;
        for (int i = 0; i < 2; i++) {
            uint8_t f = (uint8_t)(rd8(m, a0 + PS_COLL_FLAGS) & (uint8_t)~0x01u);
            uint8_t d0b = (rd8(m, G_CTRL_FLAGS) & 0x02u) ? 1u : 0u;
            wr8(m, a0 + PS_COLL_FLAGS, (uint8_t)(f | d0b));
            uint32_t a1 = rd32(m, a0 + PS_IO_CH);
            uint32_t fn = rd32(m, a0 + PS_TICK_FN);
            if (fn == PLAYER_FN_SHIP) {
                rc = Player_AliveShipTick(m, a0, a1);
            } else if (fn == 0x000C74CCu &&
                       (rd8(m, a1 + 1u) & 0x80u) != 0u &&
                       !(rd8(m, G_CTRL_FLAGS) & 0x80u) &&
                       !(rd8(m, F3_A5 + 0x08A0u) & 0x80u)) {
                wr32(m, a0 + PS_TICK_FN, 0x000C74CCu);
                rc = 0;
            } else {
                rc = -1;
            }
            if (rc != 0) return -1;
            if (rd8(m, G_P2_DISABLED) != 0u) break;
            a0 = P2_STATE_ADDR;
        }
    }
    Player_PickActive(m);
    if (Player_Hud(m) != 0) return -1;
    if ((int16_t)rd16(m, P1_STATE_ADDR + 0x4C) >= 0) return 0;
    if ((int16_t)rd16(m, P2_STATE_ADDR + 0x4C) >= 0) return 0;
    return Player_StageFail(m, P1_STATE_ADDR);
}
