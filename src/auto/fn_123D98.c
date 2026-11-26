/* ROM 0x123D98: stage-clear / warp zoom-in line effect tick (A0 = object).
 * Negates +0x14 (X) and +0x18 (Y) into scroll globals A5+0x4214/-0x5D98 and
 * A5+0x4218/-0x5D94, builds the vertical line-zoom ramp in 0x624400 via
 * 0x17671E, sets/clears line-enable bit 13 in 0x62B400 via 0x1766DA/0x1766EE,
 * and steps +0x3A by -0x444. While +0x3A.w >= 0, re-arms 0x123D98; once < 0,
 * clears PF1/line RAM (0x005956), plays sound 0x31 (0x13F238 + 0x003B66),
 * draws compressed tilemaps at 0x612020/0x612A20 (0x0053B2), clears +0x3E
 * and +0x22, and transitions to 0x122208. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../audio/sound_cmd.h"
#include "../video/sprite.h"

static uint32_t swap32(uint32_t v) { return (v << 16) | (v >> 16); }
static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x1766DA: clear bit 13 (0x2000) in words [d6w..d7w] at a0 */
static void sub_1766DA(const f3mem_t *m, uint32_t a0, int16_t d6w, int16_t d7w)
{
    while (d6w <= d7w) {
        uint32_t addr = a0 + (uint32_t)(int32_t)d7w * 2u;
        wr16(m, addr, (uint16_t)(rd16(m, addr) & ~0x2000u));
        d7w--;
    }
}

/* ROM 0x1766EE: set bit 13 (0x2000) in words [d6w..d7w] at a0 */
static void sub_1766EE(const f3mem_t *m, uint32_t a0, int16_t d6w, int16_t d7w)
{
    while (d6w <= d7w) {
        uint32_t addr = a0 + (uint32_t)(int32_t)d7w * 2u;
        wr16(m, addr, (uint16_t)(rd16(m, addr) | 0x2000u));
        d7w--;
    }
}

/* ROM 0x17671E: fill vertical line-zoom table a0[0..0xFF] around row d7w */
static void sub_17671E(const f3mem_t *m, uint32_t a0, int16_t d7w, uint32_t d3)
{
    int16_t d6w = d7w;                                   /* 0x17671E */
    uint32_t d2 = 0u;                                    /* 0x176720 */
    uint16_t flip = rd16(m, F3_A5 - 0x5D58u);
    for (;;) {
        uint16_t d0w = (uint16_t)(d6w << 1);             /* 0x176722..0x176724 */
        uint16_t d1w = (uint16_t)(d2 >> 16);             /* 0x176726..0x176728 */
        if (flip != 0u)                                  /* 0x17672A..0x17672E */
            d1w = (uint16_t)(-(int16_t)d1w);             /* 0x176732 */
        uint32_t addr = a0 + (uint32_t)(int32_t)(int16_t)d0w;
        uint16_t d4w = rd16(m, addr);                    /* 0x176734 */
        d4w = (uint16_t)((d4w & 0xFE00u) | (d1w & 0x01FFu)); /* 0x176738..0x176740 */
        wr16(m, addr, d4w);                              /* 0x176742 */
        d2 += d3;                                        /* 0x176746 */
        d6w--;                                           /* 0x176748 dbf */
        if (d6w == -1)
            break;
    }

    d7w = (int16_t)(d7w + 1);                            /* 0x17674C */
    d2 = 0u;                                             /* 0x17674E */
    while (d7w < 0x0100) {                               /* 0x176774..0x176778 */
        uint16_t d1w = (uint16_t)(d2 >> 16);             /* 0x176750..0x176752 */
        if (flip != 0u)                                  /* 0x176754..0x176758 */
            d1w = (uint16_t)(-(int16_t)d1w);             /* 0x17675C */
        uint32_t addr = a0 + (uint32_t)(int32_t)d7w * 2u;
        uint16_t d4w = rd16(m, addr);                    /* 0x17675E */
        d4w = (uint16_t)((d4w & 0xFE00u) | (d1w & 0x01FFu)); /* 0x176762..0x17676A */
        wr16(m, addr, d4w);                              /* 0x17676C */
        d2 -= d3;                                        /* 0x176770 */
        d7w++;                                           /* 0x176772 */
    }
}

int Fn_123D98(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t x = rd32(m, obj + 0x14u);                   /* 0x123D9C */
    uint32_t y = rd32(m, obj + 0x18u);                   /* 0x123DA0 */
    uint32_t d0 = rd32(m, obj + 0x3Au);                  /* 0x123DA4 */
    uint32_t d1 = y;                                     /* 0x123DA8 */
    uint32_t d2 = x;                                     /* 0x123DAA */

    uint32_t d3 = (uint32_t)(-(int32_t)d2);              /* 0x123DAC..0x123DAE */
    wr32(m, F3_A5 + 0x4214u, d3);                        /* 0x123DB0 */
    wr32(m, F3_A5 - 0x5D98u, d3);                        /* 0x123DB4 */
    d3 = (uint32_t)(-(int32_t)d1);                       /* 0x123DB8..0x123DBA */
    wr32(m, F3_A5 + 0x4218u, d3);                        /* 0x123DBC */
    wr32(m, F3_A5 - 0x5D94u, d3);                        /* 0x123DC0 */
    d1 = swap32(d1);                                     /* 0x123DC4 */

    /* 0x123DC6 saves D0..D7, A0..A2 */
    int16_t lram_off = (int16_t)rd16(m, F3_A5 - 0x5D56u);
    {
        uint32_t lram = 0x00624400u + (uint32_t)(int32_t)lram_off; /* 0x123DCA..0x123DD0 */
        int32_t d0_s = ((int32_t)d0 < 0)
            ? ~(int32_t)((uint32_t)(~(int32_t)d0) >> 2)
            : (int32_t)((uint32_t)d0 >> 2);              /* 0x123DD6 asr.l #2 */
        uint32_t step;
        if (d0_s == 0) {                                 /* 0x123DD8 */
            step = 0xFFFFFFFFu;                          /* 0x123DF2 */
        } else {
            int32_t q = 0x40000000 / d0_s;               /* 0x123DDC..0x123DE2 */
            step = (uint32_t)(-q + 0x10000);             /* 0x123DE6..0x123DE8 */
        }
        sub_17671E(m, lram, (int16_t)(uint16_t)d1, step); /* 0x123DF4..0x123DF6 */
    }
    /* 0x123DFC restores D0..D7, A0..A2 (note D3 is preserved from entry!) */
    d3 = r->d[3];

    uint16_t d4w = 0u;                                   /* 0x123E00 */
    uint16_t d5w = 0x00E7u;                              /* 0x123E02 */
    uint32_t lram_b = 0x0062B400u + (uint32_t)(int32_t)lram_off;
    if ((int32_t)d0 < 0x10000) {                         /* 0x123E06..0x123E0C */
        d0 = (uint32_t)(uint16_t)d0 * 0x00C0u;           /* 0x123E10 mulu.w */
        d0 = swap32(d0);                                 /* 0x123E14 */
        uint16_t d3w = (uint16_t)((uint16_t)d0 + (uint16_t)d1); /* 0x123E16..0x123E18 */
        d3 = setw(d3, d3w);
        if ((int16_t)d3w < 0x00E7) {                     /* 0x123E1A..0x123E1E */
            sub_1766DA(m, lram_b, (int16_t)d3w, 0x00E7); /* 0x123E36 */
            d5w = (uint16_t)(d3w - 1u);                  /* 0x123E40..0x123E42 */
        }
        d3w = (uint16_t)((uint16_t)d1 - (uint16_t)d0);   /* 0x123E44..0x123E48 */
        d3 = setw(d3, d3w);
        if ((int16_t)d3w > 0) {                          /* 0x123E4A..0x123E4C */
            sub_1766DA(m, lram_b, 0, (int16_t)d3w);      /* 0x123E64 */
            d4w = (uint16_t)(d3w + 1u);                  /* 0x123E6E..0x123E70 */
        }
    }
    sub_1766EE(m, lram_b, (int16_t)d4w, (int16_t)d5w);   /* 0x123E84 */

    /* 0x123E8E restores A0 = obj */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - 0x00000444u); /* 0x123E92 */
    if ((int16_t)rd16(m, obj + 0x3Au) >= 0) {            /* 0x123E9A..0x123EA0 */
        wr32(m, obj + 0x00u, 0x00123D98u);               /* 0x123D90 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->d[2] = d2;
        r->d[3] = d3;
        r->d[4] = setw(r->d[4], d4w);
        r->d[5] = setw(r->d[5], d5w);
        r->d[6] = 0u;
        r->d[7] = 0xFFFFFFFFu;
        return 0;                                        /* 0x123D96 */
    }

    Gfx_ClearPf1AndLineRam(m);                           /* 0x123EA4 */
    Sound_PlayCmdForced(m, Sound_PreCmdHook(m, 0x0031)); /* 0x123EAA..0x123EBA */
    Gfx_DrawCompressedTilemap(m, 0x000816ECu, 0x00612020u, 0xFFFF8000u); /* 0x1221C4..0x1221DA */
    Gfx_DrawCompressedTilemap(m, 0x000816ECu, 0x00612A20u, 0x00000000u); /* 0x1221DE..0x1221F4 */
    wr32(m, obj + 0x3Eu, 0u);                            /* 0x1221F8 */
    wr8(m, obj + 0x22u, 0u);                             /* 0x1221FC */
    wr32(m, obj + 0x00u, 0x00122208u);                   /* 0x122200 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    return 0;                                            /* 0x122206 */
}
