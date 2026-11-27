/* Superseded by src/stage/seg_13F5B8.c + seg_13DAD4.c (segments, verified on
 * captures); this whole-routine form ignores the TRAP #5 yields. */
/* ROM 0x13F5B8: UNVERIFIED (no capture): stage-end sequence. Clears object
 * groups 3..4, sets the A5-0x144C flags, starts the level jingle (0x15D0 with
 * a stage-dependent id), fills the sprite/line tables, spawns the stage's
 * object 0x13F964[stage] in groups 3..4, waits for the stage-clear flag
 * (A5-0x144B bit 3) while pacing spawn waves, then runs 0x13DAD4: the party
 * palette/scroll swarms, the "GAME OVER"-style text and the final fade. */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"
#include "../stage/stage_timeline.h"
#include "../video/sprite.h"
#include "../video/palette.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/fn_11BC6E.h"

/* Fill the 0xE8 words at base + (A5-0x5D56) with `val` (the ROM writes them
 * forward with move.w (A0)+ / dbf D7). */
static void fill_e8_words(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (unsigned i = 0; i < 0xE8u; i++)
        wr16(m, a0 + i * 2u, val);
}

/* Fixed-count `move.w #n,D7 ; subq.w #1,D7 ; L: trap #5 ; btst.b #1,A5-0x144B ;
 * bne L ; dbf D7,L`. n frame yields: scheduler timing only, no RAM effect. */
static void yield_frames(uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        /* trap #5 (frame yield) */
    }
}

/* ROM 0x13E91E: project A5+0x32EE/0x32F0 through the (0x13E9D8 or 0x13E9E0)
 * 8-byte record selected by A5-0x5D58, add the A5+0x32F2/0x32F4 offsets,
 * write the resulting pair to 0x628400 + (A5+0x3300).w, and copy the two longs
 * to A5+0x4214/0x4218. Saves and restores D0/D1/D2/A0 (no register change). */
static void pf_scroll_13E91E(const f3mem_t *m)
{
    uint32_t a0 = (rd8(m, F3_A5 - 0x5D58u) != 0u) ? 0x0013E9E0u : 0x0013E9D8u;
    f3regs_t s;
    uint16_t d1w, d0w;

    for (unsigned i = 0; i < 8u; i++) {
        s.d[i] = 0u;
        s.a[i] = 0u;
    }
    s.a[0] = a0;
    s.d[0] = rd16(m, F3_A5 + 0x32F6u);                  /* 0x13E93A */
    s.d[1] = rd16(m, F3_A5 + 0x32EEu);                  /* 0x13E93E */
    Leaf_13E994(m, &s);                                 /* 0x13E942 */
    wr32(m, F3_A5 - 0x5D98u, s.d[0]);                   /* 0x13E946 */
    d0w = rd16(m, F3_A5 + 0x32F2u);                     /* 0x13E94A */
    wr16(m, F3_A5 - 0x5D98u, (uint16_t)(rd16(m, F3_A5 - 0x5D98u) + d0w)); /* add.w D0w */

    s.d[0] = rd16(m, F3_A5 + 0x32F8u);                  /* 0x13E952 */
    s.d[1] = rd16(m, F3_A5 + 0x32F0u);                  /* 0x13E956 */
    Leaf_13E9B0(m, &s);                                 /* 0x13E95A */
    wr32(m, F3_A5 - 0x5D94u, s.d[0]);                   /* 0x13E95E */
    d0w = rd16(m, F3_A5 + 0x32F4u);                     /* 0x13E962 */
    wr16(m, F3_A5 - 0x5D94u, (uint16_t)(rd16(m, F3_A5 - 0x5D94u) + d0w)); /* add.w D0w */

    /* bfins D0,D1{0x10:0x8} on a data register: D1 bits 15..8 = D0 low byte. */
    d1w = (uint16_t)(((rd16(m, F3_A5 + 0x32F6u) & 0xFFu) << 8) |
                     (rd16(m, F3_A5 + 0x32F8u) & 0xFFu)); /* 0x13E96A..0x13E972 */
    {
        uint32_t dst = 0x00628400u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 + 0x3300u);
        wr16(m, dst, d1w);                              /* 0x13E976..0x13E980 */
    }
    wr32(m, F3_A5 + 0x4214u, rd32(m, F3_A5 - 0x5D98u)); /* 0x13E982 */
    wr32(m, F3_A5 + 0x4218u, rd32(m, F3_A5 - 0x5D94u)); /* 0x13E988 */
}

/* ROM 0x13DAD4: the stage-end party. Plays the jingle set, fills the PF1/line
 * and text tables, spawns 0x13DE92/0x13E12C/0x13DEF0 swarms, ramps the party
 * palette, fades in, waits, and finally clears PF1 and bumps A5+0x32FE. */
static void stage_end_13DAD4(const f3mem_t *m, f3regs_t *r)
{
    if (rd16(m, F3_A5 + 0x32DCu) != 0u && rd16(m, F3_A5 + 0x32DEu) != 7u)
        Sound_PlayHooked(m, 0x0014u);                   /* 0x13DAD4..0x13DAF8 */
    wr8(m, F3_A5 + 0x1D1Cu, 0xFFu);                     /* 0x13DAFC st */
    if (rd16(m, F3_A5 + 0x32DEu) == 7u)                 /* 0x13DB00 */
        Sound_PlayHooked(m, 0x0031u);                   /* 0x13DB0A */
    Sound_PlayHooked(m, 0x0017u);                       /* 0x13DB1E */
    Gfx_ClearPf1AndLineRam(m);                          /* 0x13DB32 */
    Fn_13E8A2(m, r);                                    /* 0x13DB38 */

    fill_e8_words(m, 0x0062B400u, 0x700Eu);             /* 0x13DB3E..0x13DB58 */
    fill_e8_words(m, 0x00626200u, 0x0FF9u);             /* 0x13DB5C..0x13DB76 (30 FC 0F F9) */

    Pal_QueueUpload(m, 0x000204C0u, 0x0A00u, 0x0020u);  /* 0x13DB7A..0x13DB92 */
    (void)Obj_Spawn(m, r->a[0], 0x0013DE92u, 0u, 1u, 0u); /* 0x13DB96..0x13DBAE */

    yield_frames(5u);                                   /* 0x13DBB2 */

    wr16(m, F3_A5 + 0x32EEu, rd16(m, F3_A5 + 0x32FAu)); /* 0x13DBC6 */
    wr16(m, F3_A5 + 0x32F0u, rd16(m, F3_A5 + 0x32FCu)); /* 0x13DBCC */
    wr16(m, F3_A5 + 0x32F2u, 0x0030u);                  /* 0x13DBD2 */
    wr16(m, F3_A5 + 0x32F4u, 0x0030u);                  /* 0x13DBD8 */
    wr16(m, F3_A5 + 0x32F6u, 0x0000u);                  /* 0x13DBDE */
    wr16(m, F3_A5 + 0x32F8u, 0x0080u);                  /* 0x13DBE4 */
    pf_scroll_13E91E(m);                                /* 0x13DBEA..0x13DBF6 */
    yield_frames(2u);                                   /* 0x13DBF8 trap #5 x2 */

    Gfx_DrawCompressedTilemap(m, 0x00066DC2u, 0x00612000u, 0x0200u); /* 0x13DBFC..0x13DC12 */

    {                                                   /* 0x13DC16..0x13DC3A */
        uint16_t d0w = 0x0000u;
        uint16_t d1w = 0x8000u;
        for (uint16_t d3 = 0x000Fu;; d3--) {
            d0w = (uint16_t)(d0w + 0x0F00u);            /* addi.w #0xF00 */
            d1w = (uint16_t)(d1w - 0x0780u);            /* subi.w #0x780 */
            wr16(m, F3_A5 + 0x32F6u, (uint16_t)((d0w >> 8) & 0xFFu)); /* bfextu {0x10:8} */
            wr16(m, F3_A5 + 0x32F8u, (uint16_t)((d1w >> 8) & 0xFFu));
            pf_scroll_13E91E(m);                        /* 0x13DC32 */
            /* 0x13DC38 trap #5 */
            if (d3 == 0u)
                break;
        }
    }

    {   /* 0x13DC3E movem A0/D0 around 0x145654 */
        uint32_t d0 = r->d[0], a0 = r->a[0];
        Fn_145654(m, r);                                /* 0x13DC42 */
        r->d[0] = d0;
        r->a[0] = a0;
    }
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x10u)); /* 0x13DC4C */
    (void)Obj_Spawn(m, r->a[0], 0x0013E12Cu, 0u, 7u, 0u); /* 0x13DC52..0x13DC6A */
    wr8(m, F3_A5 + 0x32FEu, 0u);                        /* 0x13DC6E sf */

    while ((int32_t)Obj_Spawn(m, r->a[0], 0x0013DEF0u, 0u, 7u, 0x0013E04Cu) > 0)
        ;                                               /* 0x13DC72..0x13DC92 */
    yield_frames(0x28u);                                /* 0x13DC94 */
    wr8(m, F3_A5 + 0x32FEu, (uint8_t)(rd8(m, F3_A5 + 0x32FEu) + 1u)); /* addq.b #1 */
    fill_e8_words(m, 0x00626200u, 0x9FD0u);             /* 0x13DCAC..0x13DCC6 */
    Fn_13DE08(m, r);                                    /* 0x13DCCA */
    (void)Pal_QueueRamp(m, 0x00016980u, 0x0013DE62u, 0x0A00u, 0x18u, 0x0Cu); /* 0x13DCCE */
    yield_frames(0x18u);                                /* 0x13DCF6 */
    Fn_13DE08(m, r);                                    /* 0x13DD0A */
    Pal_FadeIn001890(m, 0x0013DE62u, 0x0A00u, 0x00B4u, 0x0Cu); /* 0x13DD0E */

    for (uint16_t d7 = 0x0018u;; d7--) {                 /* 0x13DD30..0x13DD58 */
        while ((int32_t)Obj_Spawn(m, r->a[0], 0x0013DEF0u, 0u, 7u, 0x0013E05Cu) > 0)
            ;
        /* trap #5 */
        if (d7 == 0u)
            break;
    }
    if (rd16(m, F3_A5 + 0x32DEu) != 7u)                 /* 0x13DD5C */
        Sound_PlayHooked(m, 0x0018u);                   /* 0x13DD66 */
    Fn_13DE08(m, r);                                    /* 0x13DD7A */
    for (uint16_t d7 = 0x0018u;; d7--) {                 /* 0x13DD7E..0x13DDAA */
        while ((int32_t)Obj_Spawn(m, r->a[0], 0x0013DEF0u, 0u, 7u, 0x0013E06Cu) > 0)
            ;
        /* trap #5 */
        if (d7 == 0u)
            break;
    }
    yield_frames(0x32u);                                /* 0x13DDAA */
    if (rd16(m, F3_A5 + 0x32DEu) == 7u) {               /* 0x13DDBE */
        int16_t c = Sound_PreCmdHook(m, 0x0031);        /* 0x13DDCC */
        Sound_PlayCmdForced(m, c);                      /* 0x13DDD2 */
    }
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & 0xEFu)); /* 0x13DDDC */
    Fn_13F206(m, r);                                    /* 0x13DDE2 */
    yield_frames(0x3Cu);                                /* 0x13DDE8 */
    Gfx_ClearPf1AndLineRam(m);                          /* 0x13DDFC */
    wr8(m, F3_A5 + 0x32FEu, (uint8_t)(rd8(m, F3_A5 + 0x32FEu) + 1u)); /* 0x13DE02 */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0xFFFFu;        /* last dbf leaves D7 = -1 */
}

int Fn_13F5B8(const f3mem_t *m, f3regs_t *r)
{
    /* 0x13F5B8 move.w #2,-(SP) ; move.w #3,-(SP) ; jsr 0x1060E */
    Obj_ResetGroups(m, 3u, 2u);
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x08u));  /* bset.b #3 */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xEFu));  /* bclr.b #4 */

    {
        uint16_t d0 = 2u;
        int16_t de = (int16_t)rd16(m, F3_A5 + 0x32DEu);
        if (de > 3) {                                   /* 0x13F5DA ble */
            if (de > 6)                                 /* 0x13F5E4 ble */
                d0 = (uint16_t)(d0 + 1u);               /* 0x13F5EE */
            d0 = (uint16_t)(d0 + 1u);                   /* 0x13F5F0 */
        }
        Fx_StartA(m, (uint8_t)d0);                      /* 0x13F5F2 jsr 0x15D0 */
    }

    yield_frames(0x3Cu);                                /* 0x13F5F8 */
    fill_e8_words(m, 0x00627600u, 0xDDDDu);             /* 0x13F60C..0x13F626 */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) & 0xF7u));  /* 0x13F62A */

    {
        uint16_t stage = rd16(m, F3_A5 + 0x32DCu);      /* 0x13F630..0x13F63A */
        uint32_t fn = rd32(m, 0x0013F964u +
                           (uint32_t)((int32_t)(int16_t)stage * 4));
        (void)Obj_Spawn(m, 0x0013F964u, fn, 3u, 2u, 0u); /* 0x13F63E..0x13F652 */
    }
    wr16(m, F3_A5 + 0x3318u, 0u);                       /* 0x13F656 clr.w */

    for (;;) {                                          /* 0x13F65A */
        /* trap #5 (frame yield) */
        if (!(rd8(m, F3_A5 - 0x144Cu) & 0x10u)) {       /* 0x13F65C btst.b #4 ; bne */
            wr16(m, F3_A5 + 0x3318u,
                 (uint16_t)(rd16(m, F3_A5 + 0x3318u) + 1u)); /* 0x13F666 */
            if ((int16_t)rd16(m, F3_A5 + 0x3318u) > 0x2A30) /* 0x13F66A ble */
                Obj_11BC6E_SpawnWave(m, 0x0013F964u);   /* 0x13F674 */
        }
        if (rd8(m, F3_A5 - 0x144Bu) & 0x08u)             /* 0x13F67A beq 0x13F65A */
            break;
        return 1;                                       /* yielded; resume next frame */
    }

    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x04u)); /* 0x13F682 bset.b #2 */
    if (rd16(m, F3_A5 + 0x32DEu) == 7u)                 /* 0x13F688 */
        wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x80u)); /* 0x13F692 */

    r->a[0] = 0x0013F964u;                              /* ROM A0 at the 0x13DAD4 call */
    stage_end_13DAD4(m, r);                             /* 0x13F698 */
    return 0;
}
