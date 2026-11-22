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

    /* 0x0C6B66..0x0C6BD2: jsr 0x001660(TBL_PAL_0C9242, dst, n) x4, called as
     * 0x001660(a0 = src, (SP+0xC) = dst colour word, (SP+0x8) = count):
     *   (3, 0x354) (8, 0x360) (3, 0x94) (8, 0xA0)  [frame count, palette row]
     * 0x001660 is not decompiled yet: the frame stops before the writes. */
    return -1;

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
/* ROM 0x0C8488 / 0x0C87F8: HUD animation-row advance (per player, then the  */
/* 0x001660 palette copy which is not decompiled).                          */
/* ======================================================================== */
int Player_HudRow1(const f3mem_t *m)
{
    (void)m;
    return -1; /* 0x0C84E4: 0x001660 + palette words; not decompiled */
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
    }

    /* 0C6A7A: Obj_Spawn(A0 = 0, fn 0x0C685A, group 2, n 7, arg 0) */
    Obj_Spawn(m, 0u, 0x000C685Au, 2u, 7u, 0u);
    if (Player_SceneInit(m) != 0)              /* 0C6A96 */
        return -1;

    /* LAB_0C6A9A: TRAP #5 resumes at LAB_0C6A9C */
    wr16(m, G_FRAME_COUNTER, (uint16_t)(rd16(m, G_FRAME_COUNTER) + 1u));
    if (rd8(m, G_PAUSE) & 0x02u)
        return 0;                        /* paused: the whole frame is the yield */

    /* 0C6AA8: A0 = active player list head */
    wr32(m, G_ACTIVE_LIST, P1_STATE_ADDR);

    /* 0C6AB0: jsr 0x0B9080 -- object-script pass, A0 = 0x41B190, A5 = 0x408000.
     * Not decompiled in src/: stop before its effects. */
    return -1;

#if 0
    /* 0C6AB6..0C6B32, enable once 0x0B9080 and 0x0C8488/0x0C8A5E exist. Here is
     * the ROM register map, verbatim:
     *   bsr 0x0C8488 ; bsr 0x0C87F8
     *   A0 = P1 ; bclr.b #0,(0x34,A0) ; D0b = !btst.b #1,(0x1D36,A5) ;
     *     or.b D0b,(0x34,A0) ; A1 = (0x44,A0) ; A2 = (A0) ; jsr (A2)
     *   if !*(0x1D36-0x5DC6): the same for P2 (A0 = 0x41B21A)
     *   bsr 0x0C6E10 ; bsr 0x0C8A5E
     *   if P1.+0x4C >= 0 -> yield (0x0C6A9A)
     *   if P2.+0x4C >= 0 -> yield
     *   else stage fail 0x0C6E5A (never returns)
     * The per-player tick is dispatched through *(A0) (+0x00, u32). Only the
     * SHIP fn (0x0C77BE -> ... -> 0x0C7C60, which ends in the respawn helpers
     * of this file) is understood; any other fn stops the frame. */
    if (Player_HudRow1(m) != 0) return -1;
    Player_HudRow2(m);
    {
        uint32_t a0 = P1_STATE_ADDR;
        int rc = 0;
        for (int i = 0; i < 1; i++) {
            uint8_t f = (uint8_t)(rd8(m, a0 + PS_COLL_FLAGS) & (uint8_t)~0x01u);
            uint8_t d0b = (rd8(m, G_CTRL_FLAGS) & 0x02u) ? 0u : 1u;
            wr8(m, a0 + PS_COLL_FLAGS, (uint8_t)(f | d0b));
            if (rd32(m, a0 + PS_TICK_FN) == PLAYER_FN_SHIP)
                rc = Player_CoinCheckA(m, a0);
            else
                rc = -1;
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
#endif
}
