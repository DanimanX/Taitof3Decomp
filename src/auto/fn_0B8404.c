/* Superseded by src/stage/seg_0B8404.c (segments, verified on captures); this
 * whole-routine form ignores the TRAP #4/#5 yields. */
/* ROM 0x0B8404: UNVERIFIED (no capture): attract/title task (kernel slot 1).
 * The pre-yield init (0x0B8404 bsr 0x0B842E) reloads sound bank 0, clears the
 * sprite list and every playfield layer. The 0x0B8408 frame paints the legal
 * notice once (bit 0 of A5+0x089A gates 0x0B8442), then runs the attract
 * sub-sequences: 0x0B8956 (notice fade/tilemap), 0x0B89EA (spawns the attract
 * stage task 0x0B935A), 0x0B8E2C (a bare rts stub), 0x0B8E7A (seed the stage,
 * launch the gameplay kernel tasks, flash the prompt) and 0x0B8F7C (high
 * scores, then kill the attract tasks). */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"
#include "../kernel/fn_010420.h"
#include "../video/sprite.h"
#include "../video/palette.h"
#include "../enemy/obj_1194C2.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"

/* ---- kernel TRAP services used by the attract tasks -------------------- */
/* docs/KERNEL.md: TRAP #1 (0x001E76) spawns `entry` in `slot`, TRAP #2
 * (0x001E0E) clears task_active_mask bits, TRAP #4 (0x001F30) sleeps. The
 * context-switch/stack construction is kernel-internal, so only the confirmed
 * Work-RAM task-table bookkeeping is modelled here. */
static void sys_spawn_req(const f3mem_t *m, uint16_t slot, uint32_t entry)
{
    uint32_t id = (uint32_t)(slot & 0x0Fu);
    uint32_t parent = (uint32_t)(rd16(m, F3_A5 - 0x1906u) & 0xFFu);
    (void)entry;                                        /* kernel 0x001E76 */
    wr32(m, F3_A5 - 0x1904u, rd32(m, F3_A5 - 0x1904u) | (1u << id));
    wr32(m, F3_A5 - 0x18F8u + id * 4u, parent << 16);   /* task_status = parent */
}

static void sys_kill_mask(const f3mem_t *m, uint32_t mask)
{
    uint32_t cur = (uint32_t)(rd16(m, F3_A5 - 0x1906u) & 0x1Fu);
    uint32_t keep = ~(mask & ~(1u << cur) & ~1u);       /* protect self & slot 0 */
    wr32(m, F3_A5 - 0x1904u, rd32(m, F3_A5 - 0x1904u) & keep);
}

static void sys_sleep(const f3mem_t *m, uint16_t frames)
{
    uint32_t cur = (uint32_t)(rd16(m, F3_A5 - 0x1906u) & 0x0Fu);
    wr32(m, F3_A5 - 0x18F8u + cur * 4u,
         ((uint32_t)(0xA000u | cur) << 16) | (uint32_t)frames);
    wr32(m, F3_A5 - 0x18B8u + cur * 4u, 0u);            /* task_wakeup_val = 0 */
}

/* ROM 0x0014E8 with D0.b = 0 (index 0): copy the block-0 record of the
 * 0x16890 table ({count.w, {src_off.w, dst_off.w}*count} words) through to the
 * sound upload buffer 0x440000, source base 0x16900. This is the bank-0 half of
 * 0x0014DA; 0x14DA also runs 0x001504, so Kernel_SoundBank0_0014DA is NOT the
 * same call and cannot be used at 0x0B8ADE. Mirrors src/kernel/fn_010420.c. */
static void sound_bank0_14E8(const f3mem_t *m)
{
    uint32_t a2 = rd32(m, 0x00016890u);                 /* (a0,d0.w*8) with d0 = 0 */
    int32_t d3 = (int32_t)rd32(m, 0x00016894u);
    if (d3 < 0)                                         /* bmi */
        return;
    do {
        d3--;
        int16_t d2 = (int16_t)(rd16(m, a2) - 1u);       /* move.w (a2)+,d2 */
        a2 += 2u;
        do {
            d2--;
            uint16_t d0 = rd16(m, a2);                  /* move.w (a2)+,d0 */
            uint16_t d1 = rd16(m, a2 + 2u);             /* move.w (a2)+,d1 */
            a2 += 4u;
            wr32(m, 0x00440000u + (uint16_t)d1, rd32(m, 0x00016900u + d0));
            d0 = (uint16_t)(d0 + 4u);
            d1 = (uint16_t)(d1 + 4u);
        } while (d2 >= 0);                              /* dbra d2 */
    } while (d3 >= 0);                                  /* dbra d3 */
}

/* ROM 0x0B8442: the legal-notice text screen. The ROM word 0x1FFFFE selects
 * the line set: 3 -> only clear the layer; 2 -> twelve lines; otherwise ten.
 * The default/2 paths then sleep 0xB4 frames and clear the layer. */
static void notice_0B8442(const f3mem_t *m)
{
    static const uint32_t src_def[10] = {
        0x000B86ACu, 0x000B86CBu, 0x000B86EAu, 0x000B8709u, 0x000B8728u,
        0x000B8747u, 0x000B8766u, 0x000B8785u, 0x000B87A4u, 0x000B87C3u
    };
    static const uint32_t dst_def[10] = {
        0x0061C28Cu, 0x0061C38Cu, 0x0061C48Cu, 0x0061C58Cu, 0x0061C68Cu,
        0x0061C78Cu, 0x0061C88Cu, 0x0061C98Cu, 0x0061CA8Cu, 0x0061CB8Cu
    };
    static const uint32_t src_two[12] = {
        0x000B87E2u, 0x000B8801u, 0x000B8820u, 0x000B883Fu, 0x000B885Eu,
        0x000B887Du, 0x000B889Cu, 0x000B88BBu, 0x000B88DAu, 0x000B88F9u,
        0x000B8918u, 0x000B8937u
    };
    static const uint32_t dst_two[12] = {
        0x0061C18Cu, 0x0061C28Cu, 0x0061C38Cu, 0x0061C48Cu, 0x0061C58Cu,
        0x0061C68Cu, 0x0061C78Cu, 0x0061C88Cu, 0x0061C98Cu, 0x0061CA8Cu,
        0x0061CB8Cu, 0x0061CC8Cu
    };
    uint16_t mode = rd16(m, 0x001FFFFEu);

    if (mode == 3u) {                                   /* 0x0B844A -> 0x0B86A4 */
        Gfx_ClearAllLayers(m);
        return;
    }
    if (mode == 2u) {                                   /* 0x0B8456 -> 0x0B8562 */
        for (unsigned i = 0; i < 12u; i++)
            Text_DrawString(m, src_two[i], dst_two[i], 7u);
    } else {                                            /* 0x0B845A path */
        for (unsigned i = 0; i < 10u; i++)
            Text_DrawString(m, src_def[i], dst_def[i], 7u);
    }
    sys_sleep(m, 0x00B4u);                              /* 0x0B869A trap #4 */
    Gfx_ClearAllLayers(m);                              /* 0x0B86A4 */
}

/* ROM 0x0B8956: fade the notice out, draw its compressed tilemap, sleep,
 * fade back in and wipe the layers. */
static void notice_fade_0B8956(const f3mem_t *m)
{
    Gfx_InitLineRam(m);                                            /* 0x0B8956 */
    Pal_FadeOut001866(m, 0x000201C0u, 0x700u, 0x3Cu, 0x20u);       /* 0x0B895C */
    sys_sleep(m, 2u);                                              /* 0x0B897E */
    Gfx_DrawCompressedTilemap(m, 0x00066FACu, 0x00610118u, 0u);    /* 0x0B8988 */
    sys_sleep(m, 0x0078u);                                         /* 0x0B89A2 */
    Pal_FadeIn001890(m, 0x000201C0u, 0x700u, 0x3Cu, 0x20u);        /* 0x0B89AC */
    sys_sleep(m, 0x003Cu);                                         /* 0x0B89CE */
    Gfx_ClearAllLayers(m);                                         /* 0x0B89D8 */
    sys_sleep(m, 0x003Cu);                                         /* 0x0B89DE */
}

/* ROM 0x0B89EA: install the attract stage and print the region-specific
 * notice line; 0x1FFFFE == 2 first draws a full-screen tilemap. */
static void attract_stage_0B89EA(const f3mem_t *m)
{
    uint16_t mode = rd16(m, 0x001FFFFEu);

    if (mode == 2u) {                                              /* 0x0B89EA..0B89F2 */
        Gfx_InitLineRam(m);                                        /* 0x0B89F6 */
        Gfx_ClearAllLayers(m);                                     /* 0x0B89FC */
        Gfx_DrawCompressedTilemap(m, 0x00066DDEu, 0x00610000u, 0u); /* 0x0B8A02 */
        sys_sleep(m, 0x0078u);                                     /* 0x0B8A1C */
    }
    Gfx_ClearAllLayers(m);                                         /* 0x0B8A26 */
    sys_spawn_req(m, 2u, 0x000B935Au);                             /* 0x0B8A2C */
    sys_sleep(m, 0x0C40u);                                         /* 0x0B8A3C */

    if (mode == 3u)                                                /* 0x0B8A46 -> 0x0B8A9A */
        Text_DrawString(m, 0x000B8DE0u, 0x0061CC84u, 7u);
    else if (mode == 2u)                                           /* 0x0B8A52 -> 0x0B8A7C */
        Text_DrawString(m, 0x000B8DBAu, 0x0061CC84u, 7u);
    else                                                           /* 0x0B8A5E */
        Text_DrawString(m, 0x000B8D94u, 0x0061CC84u, 7u);
    Text_DrawString(m, 0x000B8E06u, 0x0061CD04u, 7u);              /* 0x0B8AB4 */
    sys_sleep(m, 0x012Cu);                                         /* 0x0B8ACE */
    Gfx_ClearAllLayers(m);                                         /* 0x0B8AD8 */
    sound_bank0_14E8(m);                                           /* 0x0B8ADE jsr 0x14E8 (bank-0 half only) */
}

/* ROM 0x0B8FC8: pick a random stage number and launch the gameplay kernel
 * tasks (collision, player, stage timeline, object pool, group 11), then spawn
 * object 0x11A87E in group 0x0B. */
static void stage_seed_0B8FC8(const f3mem_t *m)
{
    f3regs_t scratch;
    uint32_t v;

    wr8(m, F3_A5 + 0x08A0u, 0x81u);                    /* 0x0B8FC8 */
    wr16(m, F3_A5 + 0x1D38u, 0u);                      /* 0x0B8FCE */
    wr32(m, F3_A5 + 0x087Au, 0x00800000u);             /* 0x0B8FD4 */
    wr32(m, F3_A5 + 0x087Eu, 0x00800000u);             /* 0x0B8FDC */
    v = Rng_NextMem(m) & 0xFFFFu;                      /* 0x0B8FE4..0B8FEE */
    wr16(m, F3_A5 + 0x32DCu, (uint16_t)(v % 6u));      /* 0x0B8FF0 divu/swap/store */

    for (unsigned i = 0; i < 8u; i++) {                 /* scratch regs for 0x13CAAE */
        scratch.d[i] = 0u;
        scratch.a[i] = 0u;
    }
    Leaf_13CAAE(m, &scratch);                          /* 0x0B8FFA */
    sys_spawn_req(m, 2u, 0x00144B60u);                 /* 0x0B9000 */
    sys_spawn_req(m, 3u, 0x000C63E8u);                 /* 0x0B9010 */
    sys_spawn_req(m, 5u, 0x000C6A64u);                 /* 0x0B9020 */
    sys_spawn_req(m, 6u, 0x0013F2C6u);                 /* 0x0B9030 */
    sys_spawn_req(m, 4u, 0x00010488u);                 /* 0x0B9040 */
    sys_spawn_req(m, 0x0Eu, 0x00010502u);              /* 0x0B9050 */
    /* 0x0B9060 trap #5 (frame yield) */
    Obj_Spawn(m, 0u, 0x0011A87Eu, 0x0Bu, 1u, 0u);      /* 0x0B9062..0x0B907A */
}

/* ROM 0x0B8E7A: flash the "press start" prompt for 13 frames, then stop all
 * sound and set the A5-0x144C bit 1 flag. */
static void attract_prompt_0B8E7A(const f3mem_t *m)
{
    stage_seed_0B8FC8(m);                              /* 0x0B8E7A */

    for (unsigned n = 0; n < 0x0Du; n++) {             /* 0x0B8E80 move.w #0xC,D1; dbra */
        sys_sleep(m, 0x001Eu);                         /* 0x0B8E84 trap #4 */
        Text_DrawString(m, 0x000B8F4Cu, 0x0061C71Cu, 8u); /* 0x0B8E8E */
        Text_DrawString(m, 0x000B8F70u, 0x0061C71Cu, 8u); /* 0x0B8EB2 */
        if (rd8(m, F3_A5 - 0x5DC1u) != 0u)             /* 0x0B8ED6 tst.b ; bne */
            Text_DrawString(m, 0x000B8F64u, 0x0061C71Cu, 7u); /* 0x0B8EFC */
        else
            Text_DrawString(m, 0x000B8F58u, 0x0061C71Cu, 7u); /* 0x0B8EDE */
        Text_DrawString(m, 0x000B8F70u, 0x0061C71Cu, 8u); /* 0x0B8F20 */
    }
    Sound_StopAll(m);                                  /* 0x0B8F3E */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x02u)); /* bset.b #1 */
}

/* ROM 0x0B8F7C: show the high-score table for 0x12C frames, kill the attract
 * tasks (mask 0x7FFC), wipe sprites/layers and queue the boot palette. */
static void attract_highscores_0B8F7C(const f3mem_t *m)
{
    Stage_DrawHighScores13D94C(m);                     /* 0x0B8F7C */
    for (unsigned n = 0; n < 0x12Cu; n++) {            /* 0x0B8F86 move.w #0x12C,D7; dbra */
        /* 0x0B8F88 trap #5 (frame yield) */
    }
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xFDu)); /* bclr.b #1 */
    sys_kill_mask(m, 0x00007FFCu);                     /* 0x0B8F94 */
    Gfx_ClearAllLayers(m);                             /* 0x0B8F9E */
    Sprite_ClearAll(m);                                /* 0x0B8FA4 */
    Pal_QueueUpload(m, 0x0001FB40u, 0u, 1u);           /* 0x0B8FAA..0x0B8FC2 */
}

int Fn_0B8404(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    /* 0x0B8404 bsr.w 0x0B842E */
    Kernel_SoundBank0_0014DA(m);                       /* 0x0B842E jsr 0x14DA */
    Gfx_ClearAllLayers(m);                             /* 0x0B8434 jsr 0x5A12 */
    Sprite_ClearAll(m);                                /* 0x0B843A jsr 0x117E */
    /* 0x0B8440 rts of the init; 0x0B8408 trap #5 is the first frame yield */
    return 0;
}

void TaskAttract_Frame(const f3mem_t *m)
{
    /* 0x0B8408 trap #5 (start of a frame) */
    uint8_t old = rd8(m, F3_A5 + 0x089Au);

    wr8(m, F3_A5 + 0x089Au, (uint8_t)(old | 0x01u));  /* 0x0B840A bset.b #0 */
    if (!(old & 0x01u))                                /* 0x0B8410 bne 0x0B8418 */
        notice_0B8442(m);                              /* 0x0B8414 */
    notice_fade_0B8956(m);                             /* 0x0B8418 */
    attract_stage_0B89EA(m);                           /* 0x0B841C */
    /* 0x0B8420 bsr 0x0B8E2C: that entry is a bare rts (no effect) */
    attract_prompt_0B8E7A(m);                          /* 0x0B8424 */
    attract_highscores_0B8F7C(m);                      /* 0x0B8428 */
    /* 0x0B842C bra.b 0x0B8408: next frame */
}
