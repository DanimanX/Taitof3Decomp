/* ROM 0x122208: boss/background fade-out and line-wobble tick (A0 = object).
 * When parent (+0x08)->+0x59 is non-zero, frees A0 (0x0105EC). While +0x3E.w
 * >= 0, updates line fade RAM (0x626200 + (-0x5D56,A5)), steps +0x3A by
 * 0x10000, runs line wobble 0x176992 on 0x62A400 + (-0x5D56,A5), decrements
 * +0x3E.l by 0x199, and clears bit 2 of A5+0x30EE. When that bit is still set,
 * plays sound 0x12, clears two playfield rects via 0x0054AC, updates the
 * target object at A5+0x30DE, and advances state to 0x122334; else re-arms
 * 0x122208. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x176992: fill line RAM entries [d7w..d6w] from sine table 0x027E40 */
static void sub_176992(const f3mem_t *m, f3regs_t *r, uint32_t a0,
                       uint32_t d0, uint32_t d1, uint16_t d2w,
                       uint16_t d6w, uint16_t d7w)
{
    uint32_t a1 = 0x00027E40u;                           /* 0x176992 */
    wr32(m, F3_A5 + 0x4298u, d0);                        /* 0x176998 */
    uint16_t d5w = 0x003Fu;                              /* 0x17699C */
    uint32_t d3 = 0u;                                    /* 0x1769A0 */
    a0 += (uint32_t)(int32_t)(int16_t)d7w * 2u;          /* 0x1769A2 */
    for (;;) {
        d3 = (d3 & 0xFFFFFF00u) | rd8(m, F3_A5 + 0x4299u); /* 0x1769A6 */
        int16_t s = (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)(uint16_t)d3 * 2u); /* 0x1769AA */
        int32_t prod = (int32_t)s * (int32_t)(int16_t)d2w; /* 0x1769AE */
        int32_t shifted = (prod < 0)
            ? ~(int32_t)((uint32_t)(~prod) >> 8)
            : (int32_t)((uint32_t)prod >> 8);            /* 0x1769B0 asr.l #8 */
        uint16_t outw = (uint16_t)((uint16_t)(uint32_t)shifted ^ d5w); /* 0x1769B2 */
        d0 = ((uint32_t)shifted & 0xFFFF0000u) | outw;
        wr16(m, a0, outw);                               /* 0x1769B4 */
        a0 += 2u;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d1); /* 0x1769B6 */
        d7w = (uint16_t)(d7w + 1u);                      /* 0x1769BA */
        if ((int16_t)d7w > (int16_t)d6w)                 /* 0x1769BC..0x1769BE */
            break;
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
}

int Fn_122208(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x122208 */
    r->a[1] = a1;
    if (rd8(m, a1 + 0x59u) != 0u) {                      /* 0x12220C..0x122210 */
        Obj_Free(m, obj);                                /* 0x122630 (preserves D0,A0,A1) */
        return 0;                                        /* 0x12263A */
    }

    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {            /* 0x122214..0x122218 */
        int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u);
        {
            uint32_t lram = 0x00626200u + (uint32_t)(int32_t)off; /* 0x12222C..0x122232 */
            uint16_t d0w = 0x0008u;                      /* 0x122220 */
            uint16_t d1w = (uint16_t)(rd16(m, obj + 0x3Eu) & 0x000Fu); /* 0x122224/0x122246 */
            if ((int16_t)d1w > 8)                        /* 0x12224A..0x122252 */
                d1w = 8u;
            uint16_t c0 = (uint16_t)(((d0w << 8) + (uint16_t)(8u - d0w)) << 4);
            uint16_t c1 = (uint16_t)((d1w << 8) + (uint16_t)(8u - d1w));
            uint16_t fill = (uint16_t)~(c0 | c1);        /* 0x12226C..0x12226E */
            for (int16_t d7 = 0x00E7; d7 >= 0; d7--)     /* 0x122270..0x12227C */
                wr16(m, lram + (uint32_t)d7 * 2u, fill);
        }
        /* 0x122280 restores D0..D7, A0 = obj */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + 0x00010000u); /* 0x122284 */

        /* 0x12228C..0x1222B4: A0 becomes 0x62A400 + (-0x5D56,A5); D0 is read
         * from (0x3A,A0) on that line-RAM pointer before 0x176992 runs. */
        uint32_t lram2 = 0x0062A400u + (uint32_t)(int32_t)off;
        uint32_t d0 = rd32(m, lram2 + 0x3Au);            /* 0x12229E */
        sub_176992(m, r, lram2, d0, 0x00040000u, 6u, 0x00E7u, 0u); /* 0x1222B4 */

        /* 0x1222BA restores A0 = obj */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - 0x00000199u); /* 0x1222BE */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x20u)); /* 0x1222C6 */
    }

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x20u) == 0u) {       /* 0x1222CC..0x1222D2 */
        wr32(m, obj + 0x00u, 0x00122208u);               /* 0x122200 */
        return 0;                                        /* 0x122206 */
    }

    Sound_PlayHooked(m, 0x0012u);                        /* 0x1222D6..0x1222E6 */
    Video_ClearRect(m, 0x00081778u, 0x00612000u);        /* 0x1222EA..0x1222FC */
    Video_ClearRect(m, 0x00081778u, 0x00612A00u);        /* 0x122300..0x122312 */
    a1 = rd32(m, F3_A5 + 0x30DEu);                       /* 0x122316 */
    r->a[1] = a1;
    wr32(m, a1 + 0x50u, rd32(m, a1));                    /* 0x12231A */
    wr32(m, a1, 0x001226A8u);                            /* 0x12231E */
    wr16(m, obj + 0x06u, 0x000Au);                       /* 0x122324 (Z=0, 0x12232A beq folded) */
    wr32(m, obj + 0x00u, 0x00122334u);                   /* 0x12232C */
    return 0;                                            /* 0x122332 */
}
