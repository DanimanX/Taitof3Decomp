/* ROM 0x1221C4: decode the packed tile block at ROM 0x816EC twice into the
 * playfield RAM at 0x612020 and 0x612A20 (flags 0x8000 / 0), then re-arm the
 * object at A0: +0x3E = 0, +0x22 = 0, (A0) = 0x122208. */
#include "fn_auto.h"

/* 0x53B2: run-length unpacker. src = packed stream, dest = 32-bit tile RAM
 * base, flags bits 31..30 select the walk direction (0x5352 table) and bits
 * 31..25 are XORed into the decoded longs (0x53DE). Per cell the ROM stores a
 * long at a1 + d4.w; a1 walks 0x80-byte rows, d4.w walks 4-byte cells. */
static int unpack_block(const f3mem_t *m, uint32_t src, uint32_t dest, uint32_t flags)
{
    uint32_t a0 = src;
    uint32_t a1 = dest;
    uint32_t d0 = 0, d2 = 0, d3 = flags, d4, d5, d6 = dest, d7;
    uint16_t v;
    int armed = 0;      /* a2: 0 = 0x542A command loop, 1..4 = run bodies  */

    /* 0x53BA..0x53C4: header -> d7 = (rows-1)<<16 | (cols-1) */
    d7 = (uint16_t)(rd8(m, a0) - 1u);
    a0 += 1u;
    d7 = ((d7 & 0xFFFFu) << 16) | (uint16_t)(rd8(m, a0) - 1u);
    a0 += 1u;

    /* 0x53D0..0x53DC: mode = flags bits 31..30, run the 0x5352 handler */
    switch ((d3 >> 30) & 3u) {
    case 0:                                            /* 0x535A */
        d5 = 0x00800000u | (d6 & 0xFFFFu);
        d6 = ((d6 & 0xFFFFu) << 16) | 0x0004u;
        break;
    case 1:                                            /* 0x536A */
        d5 = 0x00800000u | (d6 & 0xFFFFu);
        d0 = (uint16_t)((uint16_t)d7 << 2);            /* 4 * cols */
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + (uint16_t)d0);
        d6 = (d6 << 16) | (d6 >> 16);
        d6 = (d6 & 0xFFFF0000u) | 0xFFFCu;
        break;
    case 2:                                            /* 0x5380 */
        d5 = 0xFF800000u | (d6 & 0xFFFFu);
        d0 = (d7 >> 9) & 0xFFFu;                       /* bfextu d7{11:12} */
        d5 = (d5 & 0xFFFF0000u) | (uint16_t)((uint16_t)d5 + (uint16_t)d0);
        d6 = (d6 << 16) | (d6 >> 16);
        d6 = (d6 & 0xFFFF0000u) | 0x0004u;
        break;
    default:                                           /* 0x5396 */
        d5 = 0xFF800000u | (d6 & 0xFFFFu);
        d0 = (d7 >> 9) & 0xFFFu;
        d5 = (d5 & 0xFFFF0000u) | (uint16_t)((uint16_t)d5 + (uint16_t)d0);
        d0 = (uint16_t)((uint16_t)d7 << 2);
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + (uint16_t)d0);
        d6 = (d6 << 16) | (d6 >> 16);
        d6 = (d6 & 0xFFFF0000u) | 0xFFFCu;
        break;
    }

    d3 &= 0xFE000000u;                                 /* 0x53DE */
    d4 = (d7 & 0xFFFFu) << 16;                         /* 0x53E8: d4 high = cols-1 */

row_start:                                             /* 0x53F6 */
    d4 = (d4 & 0xFFFF0000u) | ((d6 >> 16) & 0xFFFFu);
    d5 = (d5 & 0xFFFF0000u) | ((d5 & 0xFFFFu) & 0x0F80u);
    a1 = (a1 & 0xFFFFF000u) + (d5 & 0xFFFFu);

    for (;;) {                                         /* one tile row */
        /* 0x540E: next cell of this row */
        d4 = (d4 & 0xFFFF0000u) | ((d4 & 0xFFFFu) & 0x007Cu);

        if (armed == 0) {                              /* 0x542A */
            uint16_t cmd = rd16(m, a0);
            a0 += 2u;
            if (cmd != 0) {
                uint32_t sel = (uint32_t)(cmd >> 8);   /* bfextu d0{16:8} */
                d0 = (d0 & 0xFFFF0000u) | (cmd & 0xFFu);
                switch (sel) {
                case 1:                                /* 0x5452 -> 0x5456 */
                    armed = 1;
                    break;
                case 2:                                /* 0x5464: one long, repeated */
                case 3:                                /* 0x547A: +1 per cell */
                case 4:                                /* 0x5492: -1 per cell */
                    d2 = rd32(m, a0) ^ d3;
                    a0 += 4u;
                    armed = (int)sel;
                    break;
                default:
                    /* 0x544A reads table 0x5438: selector 0 yields 0x60D8 ->
                     * 0x543A+0x60D8 = 0xB512 (ROM 0xFFFF filler), selectors
                     * 5.. read the code bytes after the table. Neither is a
                     * decodable instruction stream, so the ROM traps there. */
                    return -1;
                }
            } else {                                   /* 0x5430 literal long */
                d2 = rd32(m, a0) ^ d3;
                a0 += 4u;
                wr32(m, a1 + (uint32_t)(int32_t)(int16_t)(d4 & 0xFFFFu), d2);
            }
        }

        if (armed != 0) {                              /* the a2 run body */
            uint32_t ea = a1 + (uint32_t)(int32_t)(int16_t)(d4 & 0xFFFFu);
            switch (armed) {
            case 1:                                    /* 0x5456 clr.l */
                wr32(m, ea, 0u);
                break;
            case 2:                                    /* 0x546C */
                wr32(m, ea, d2);
                break;
            case 3:                                    /* 0x5482 */
                wr32(m, ea, d2);
                d2 = (d2 & 0xFFFF0000u) | (uint16_t)((uint16_t)d2 + 1u);
                break;
            default:                                   /* 0x549A */
                wr32(m, ea, d2);
                d2 = (d2 & 0xFFFF0000u) | (uint16_t)((uint16_t)d2 - 1u);
                break;
            }
            /* dbra d0 -> 0x5412 while the run lasts, else a2 = 0x542A */
            v = (uint16_t)((uint16_t)d0 - 1u);
            d0 = (d0 & 0xFFFF0000u) | v;
            if (v == 0xFFFFu)
                armed = 0;
        }

        /* 0x5412 */
        d4 = (d4 & 0xFFFF0000u) | (uint16_t)((uint16_t)d4 + (uint16_t)d6);
        v = (uint16_t)((uint16_t)d7 - 1u);              /* dbra d7 -> 0x540E */
        d7 = (d7 & 0xFFFF0000u) | v;
        if (v != 0xFFFFu)
            continue;

        /* 0x5418: end of tile row */
        d5 = (d5 & 0xFFFF0000u) | (uint16_t)((uint16_t)d5 + (uint16_t)(d5 >> 16));
        d7 = (d7 << 16) | (d7 >> 16);                   /* 0x541E */
        v = (uint16_t)((uint16_t)d7 - 1u);              /* dbra d7 over rows */
        d7 = (d7 & 0xFFFF0000u) | v;
        if (v == 0xFFFFu)
            return 0;
        d7 = (d7 << 16) | (d7 >> 16);                   /* 0x53EE */
        d7 = (d7 & 0xFFFF0000u) | ((d4 >> 16) & 0xFFFFu);
        goto row_start;
    }
}

int Fn_1221C4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t sp = r->a[7];

    /* 0x1221C4: word 0x8000, pea 0x612020, long 0x816EC. 0x53B2 reads its
     * third argument as a LONG at SP+0xC = SP-2 here, so the flags are
     * 0x8000 | the caller's stack word above SP (0x0001 in the capture):
     * d3 = 0x80000001 -> walk mode 0x5380 and XOR bit 31 into every long. */
    wr16(m, sp - 2u, 0x8000u);
    wr32(m, sp - 6u, 0x00612020u);
    wr32(m, sp - 10u, 0x000816ECu);
    wr32(m, sp - 14u, 0x001221DAu);                  /* jsr return address */
    if (unpack_block(m, rd32(m, sp - 10u), rd32(m, sp - 6u), rd32(m, sp - 2u)) < 0)
        return -1;

    /* 0x1221DE: word 0x0000, pea 0x612A20, long 0x816EC */
    wr16(m, sp - 2u, 0x0000u);
    wr32(m, sp - 6u, 0x00612A20u);
    wr32(m, sp - 10u, 0x000816ECu);
    wr32(m, sp - 14u, 0x001221F4u);
    if (unpack_block(m, rd32(m, sp - 10u), rd32(m, sp - 6u), rd32(m, sp - 2u)) < 0)
        return -1;

    wr32(m, a0 + 0x3Eu, 0u);                            /* 0x1221F8 clr.l */
    wr8(m, a0 + 0x22u, 0u);                             /* 0x1221FC sf */
    wr32(m, a0 + 0x00u, 0x00122208u);                   /* 0x122200 re-arm */

    /* 0x122206 rts: every register is untouched (0x53B2 saves d0-d7/a0-a2 and
     * the tail only writes memory), so r stays as it came in. */
    return 0;
}
