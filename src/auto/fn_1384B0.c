/* ROM 0x1384B0: scrolling playfield tilemap streamer (A0 = object, +0x08 =
 * parent, +0x22 = layer index 0..3). Frees self (0x0105EC) if parent->+0x59
 * != 0. Writes negated scroll coordinates from (+0x50/+0x14, +0x52/A5+0x30D6)
 * into the two scroll register pairs in table 0x1385E2[+0x22*4]. When scroll
 * word (A1) crosses a tile-column half-boundary vs +0x06, draws two 8-row
 * vertical strips via 0x1385B2 -> 0x0053B2 from descriptor table +0x0C (or
 * fallback 0x138632 when column < A5+0x32D8). Finally syncs +0x06 = (A1),
 * copies A5+0x30D2/+0x30D6 to +0x14/+0x18, and re-arms 0x1384B0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x1385B2: draw compressed tilemap strip (d2 bit 31 -> 0x02000000 attr flag) */
static void sub_1385B2(const f3mem_t *m, uint32_t a1, int16_t d0w, int16_t d1w, uint32_t d2)
{
    uint32_t flags = ((d2 >> 31) & 1u) ? 0x02000000u : 0u;
    uint32_t dst = a1 + (uint32_t)((int32_t)d0w * 4 + (int32_t)d1w * 0x80);
    uint32_t src = d2 & 0x7FFFFFFFu;
    Gfx_DrawCompressedTilemap(m, src, dst, flags);
}

int Fn_1384B0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x1384B0 */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                      /* 0x1384B4..0x1384B8 */
        Obj_Free(m, obj);                                /* 0x1385A6 (preserves D0,A0,A1) */
        return 0;
    }

    {
        uint16_t off = (uint16_t)(rd16(m, obj + 0x22u) << 4); /* 0x1384BC..0x1384C0 */
        uint32_t tbl = 0x001385E2u + (uint32_t)(int32_t)(int16_t)off; /* 0x1384C2..0x1384C8 */

        uint16_t xw = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x14u)); /* 0x1384CC..0x1384D0 */
        uint32_t xv = (uint32_t)(-(int32_t)((uint32_t)xw << 16)); /* 0x1384D4..0x1384D8 */
        wr32(m, rd32(m, tbl), xv); tbl += 4u;            /* 0x1384DA..0x1384DC */
        wr32(m, rd32(m, tbl), xv); tbl += 4u;            /* 0x1384DE..0x1384E0 */

        uint16_t yw = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, F3_A5 + 0x30D6u)); /* 0x1384E2..0x1384E6 */
        d0 = (uint32_t)(-(int32_t)((uint32_t)yw << 16)); /* 0x1384EA..0x1384EE */
        wr32(m, rd32(m, tbl), d0); tbl += 4u;            /* 0x1384F0..0x1384F2 */
        wr32(m, rd32(m, tbl), d0);                       /* 0x1384F4..0x1384F6 */
    }

    {
        uint16_t off = (uint16_t)(rd16(m, obj + 0x22u) << 4); /* 0x1384F8..0x1384FC */
        a1 = rd32(m, 0x001385E2u + (uint32_t)(int32_t)(int16_t)off); /* 0x1384FE..0x138504 */
        uint16_t cur = rd16(m, a1);                      /* 0x138508 */
        uint16_t prev = rd16(m, obj + 0x06u);            /* 0x13850C */
        d0 = setw(d0, (uint16_t)(cur >> 7));
        d1 = setw(d1, (uint16_t)(prev >> 7));
        if ((uint16_t)d0 == (uint16_t)d1) {              /* 0x138512..0x138514 */
            uint8_t s0 = ((int16_t)(cur & 0x007Fu) > 0x20) ? 0xFFu : 0x00u; /* 0x138518..0x138522 */
            uint8_t s1 = ((int16_t)(prev & 0x007Fu) > 0x20) ? 0xFFu : 0x00u; /* 0x138524..0x138530 */
            d0 = (d0 & 0xFFFFFF00u) | s0;
            d1 = (d1 & 0xFFFFFF00u) | s1;
            if (s0 != s1) {                              /* 0x138532..0x138534 */
                uint32_t a2 = rd32(m, obj + 0x0Cu);      /* 0x138538 */
                int16_t col = (int16_t)cur >> 7;         /* 0x13853C..0x13853E asr.w #7 */
                int16_t d2w = col;                       /* 0x138540 */
                int16_t idx = (s1 == 0u) ? (int16_t)(col + 3) : (int16_t)(col - 1); /* 0x138542..0x13854E */
                int16_t mod = (int16_t)rd16(m, obj + 0x42u); /* 0x138552 divs.w */
                int16_t rem = (mod != 0) ? (int16_t)((int32_t)idx % mod) : idx;
                if (rem < 0)                             /* 0x138558..0x13855E */
                    rem = (int16_t)(rem + mod);
                if (rem >= (int16_t)rd16(m, F3_A5 + 0x32D8u)) /* 0x138562..0x138566 */
                    a2 += (uint32_t)(int32_t)rem * 8u;   /* 0x13856A */
                else
                    a2 = 0x00138632u;                    /* 0x138572 */

                uint16_t layer = rd16(m, obj + 0x22u);   /* 0x138576 */
                uint32_t vram = rd32(m, 0x00138622u + (uint32_t)(int32_t)(int16_t)layer * 4u); /* 0x13857A */
                int16_t strip = (int16_t)(((d2w + 3) & 3) << 3); /* 0x138582..0x13858A */
                sub_1385B2(m, vram, strip, 0, rd32(m, a2));      /* 0x138590..0x138592 */
                sub_1385B2(m, vram, strip, 8, rd32(m, a2 + 4u)); /* 0x138596..0x13859C */
                d0 = setw(d0, (uint16_t)strip);
                d1 = setw(d1, 8u);
            }
        }
    }

    /* 0x1385A0 -> 0x138488 */
    {
        uint16_t off = (uint16_t)(rd16(m, obj + 0x22u) << 4); /* 0x138488..0x13848C */
        d0 = setw(d0, off);
        a1 = rd32(m, 0x001385E2u + (uint32_t)(int32_t)(int16_t)off); /* 0x13848E..0x138494 */
        wr16(m, obj + 0x06u, rd16(m, a1));               /* 0x138498 */
        wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));  /* 0x13849C */
        wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));  /* 0x1384A2 */
        wr32(m, obj + 0x00u, 0x001384B0u);               /* 0x1384A8 */
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;                                            /* 0x1384AE */
}
