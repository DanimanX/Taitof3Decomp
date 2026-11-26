/* ROM 0x122384: boss/stage line-scroll perspective & mosaic intro tick (A0 = object).
 * Frees self (0x0105EC) if parent (+0x08)->+0x59 != 0. Otherwise computes mosaic
 * from +0x3E into 0x626200, writes negated (+0x14, +0x18) into scroll globals
 * A5+0x4214/-0x5D98 and A5+0x4218/-0x5D94, builds perspective ramp in 0x624400
 * from +0x3A (0x17671E), sets/clears line-enable bit 13 in 0x62B400 (0x1766DA /
 * 0x1766EE), fills sine wave in 0x62A400 (0x176992), scrolls +0x14 right toward
 * -0x0A, unpacks tilemaps 0x081778 to 0x612000/0x612A00 once +0x14 > -0xA0,
 * ramps +0x3E to 6 and +0x3A to 4, then sets +0x06=0xB4, +0x3A=0, and arms
 * 0x1225B4. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
}

/* ROM 0x1766DA: clear bit 13 (0x2000) in words [d6w..d7w] at a0 */
static void sub_1766DA(const f3mem_t *m, uint32_t a0, int16_t d6w, int16_t d7w)
{
    do {
        uint32_t addr = a0 + (uint32_t)(int32_t)d7w * 2u;
        wr16(m, addr, (uint16_t)(rd16(m, addr) & ~0x2000u));
        d7w--;
    } while (d6w <= d7w);
}

/* ROM 0x1766EE: set bit 13 (0x2000) in words [d6w..d7w] at a0 */
static void sub_1766EE(const f3mem_t *m, uint32_t a0, int16_t d6w, int16_t d7w)
{
    do {
        uint32_t addr = a0 + (uint32_t)(int32_t)d7w * 2u;
        wr16(m, addr, (uint16_t)(rd16(m, addr) | 0x2000u));
        d7w--;
    } while (d6w <= d7w);
}

/* ROM 0x17671E: fill vertical line-zoom table a0[0..0xFF] around row d7w */
static void sub_17671E(const f3mem_t *m, uint32_t a0, int16_t d7w, uint32_t d3)
{
    int16_t d6w = d7w;
    uint32_t d2 = 0u;
    uint16_t flip = rd16(m, F3_A5 - 0x5D58u);
    for (;;) {
        uint16_t d0w = (uint16_t)(d6w << 1);
        uint16_t d1w = (uint16_t)(d2 >> 16);
        if (flip != 0u)
            d1w = (uint16_t)(-(int16_t)d1w);
        uint32_t addr = a0 + (uint32_t)(int32_t)(int16_t)d0w;
        uint16_t d4w = rd16(m, addr);
        d4w = (uint16_t)((d4w & 0xFE00u) | (d1w & 0x01FFu));
        wr16(m, addr, d4w);
        d2 += d3;
        d6w--;
        if (d6w == -1)
            break;
    }

    d7w = (int16_t)(d7w + 1);
    d2 = 0u;
    while (d7w < 0x0100) {
        uint16_t d1w = (uint16_t)(d2 >> 16);
        if (flip != 0u)
            d1w = (uint16_t)(-(int16_t)d1w);
        uint32_t addr = a0 + (uint32_t)(int32_t)d7w * 2u;
        uint16_t d4w = rd16(m, addr);
        d4w = (uint16_t)((d4w & 0xFE00u) | (d1w & 0x01FFu));
        wr16(m, addr, d4w);
        d2 -= d3;
        d7w++;
    }
}

int Fn_122384(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);                 /* 0x122384 */
    r->a[1] = parent;

    if (rd8(m, parent + 0x59u) != 0u) {                     /* 0x122388 */
        Obj_Free(m, obj);                                   /* 0x12262E */
        return 0;
    }

    int16_t lram_off = (int16_t)rd16(m, F3_A5 - 0x5D56u);

    /* 0x122394..0x1223F4: D0..D7/A0 saved & restored around mosaic fill */
    {
        uint32_t lram = 0x00626200u + (uint32_t)(int32_t)lram_off;
        uint16_t a = 0u;
        uint16_t b = (uint16_t)(rd16(m, obj + 0x3Eu) & 0x0Fu);
        if ((int16_t)b > 8)
            b = 8u;
        uint16_t v0 = (uint16_t)(((a << 8) + (uint16_t)(8u - a)) << 4);
        uint16_t v1 = (uint16_t)((b << 8) + (uint16_t)(8u - b));
        uint16_t fill = (uint16_t)~(v0 | v1);
        for (uint32_t i = 0; i < 0xE8u; i++)
            wr16(m, lram + i * 2u, fill);
    }

    uint32_t x = rd32(m, obj + 0x14u);                      /* 0x1223F8 */
    uint32_t y = rd32(m, obj + 0x18u);                      /* 0x1223FC */
    uint32_t d0 = rd32(m, obj + 0x3Au);                     /* 0x122400 */
    uint32_t d1 = y;                                        /* 0x122404 */
    uint32_t d2 = x;                                        /* 0x122406 */

    {
        uint32_t nx = (uint32_t)(-(int32_t)d2);
        wr32(m, F3_A5 + 0x4214u, nx);                       /* 0x12240C */
        wr32(m, F3_A5 - 0x5D98u, nx);                       /* 0x122410 */
        uint32_t ny = (uint32_t)(-(int32_t)d1);
        wr32(m, F3_A5 + 0x4218u, ny);                       /* 0x122418 */
        wr32(m, F3_A5 - 0x5D94u, ny);                       /* 0x12241C */
    }
    d1 = swap32(d1);                                        /* 0x122420 */

    /* 0x122422..0x122458: D0..D7/A0..A2 saved & restored around 0x17671E */
    {
        uint32_t lram = 0x00624400u + (uint32_t)(int32_t)lram_off;
        int32_t d0_s = ((int32_t)d0 < 0)
            ? ~(int32_t)((uint32_t)(~(int32_t)d0) >> 2)
            : (int32_t)((uint32_t)d0 >> 2);                 /* 0x122432 asr.l #2 */
        uint32_t step = (d0_s == 0)
            ? 0xFFFFFFFFu
            : (uint32_t)(-(0x40000000 / d0_s) + 0x10000);   /* 0x122438..0x122444 */
        sub_17671E(m, lram, (int16_t)(uint16_t)d1, step);   /* 0x122452 */
    }

    uint16_t d4w = 0u;                                      /* 0x12245C */
    uint16_t d5w = 0x00E7u;                                 /* 0x12245E */
    if ((int32_t)d0 < 0x10000) {                            /* 0x122462 */
        uint32_t prod = (uint32_t)(uint16_t)d0 * 0x00C0u;   /* 0x12246C */
        d0 = swap32(prod);                                  /* 0x122470 */
        int16_t d3w = (int16_t)((uint16_t)d0 + (uint16_t)d1); /* 0x122472..0x122474 */
        uint32_t lram_b4 = 0x0062B400u + (uint32_t)(int32_t)lram_off;
        if (d3w < 0x00E7) {                                 /* 0x122476 */
            sub_1766DA(m, lram_b4, d3w, 0x00E7);            /* 0x122492 */
            d5w = (uint16_t)(d3w - 1);                      /* 0x12249C..0x12249E */
        }
        d3w = (int16_t)((uint16_t)d1 - (uint16_t)d0);       /* 0x1224A0..0x1224A4 */
        if (d3w > 0) {                                      /* 0x1224A6 */
            sub_1766DA(m, lram_b4, 0, d3w);                 /* 0x1224C0 */
            d4w = (uint16_t)(d3w + 1);                      /* 0x1224CA..0x1224CC */
        }
    }

    {
        uint32_t lram_b4 = 0x0062B400u + (uint32_t)(int32_t)lram_off;
        sub_1766EE(m, lram_b4, (int16_t)d4w, (int16_t)d5w); /* 0x1224E0 */
    }

    /* 0x1224EA..0x122514: 0x176992 sine wave into 0x62A400 + lram_off */
    {
        uint32_t a0_p = 0x0062A400u + (uint32_t)(int32_t)lram_off;
        wr32(m, F3_A5 + 0x4298u, 0u);
        uint32_t last_d0 = 0u;
        for (int16_t row = 0; row <= 0x00E7; row++) {
            uint8_t idx = rd8(m, F3_A5 + 0x4299u);
            int16_t sv = (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
            int32_t prod = (int32_t)sv * 6;
            int32_t shifted = (prod < 0)
                ? ~(int32_t)((uint32_t)(~prod) >> 8)
                : (int32_t)((uint32_t)prod >> 8);
            uint32_t val32 = (uint32_t)(shifted ^ 0x3F);
            last_d0 = val32;
            wr16(m, a0_p, (uint16_t)val32);
            a0_p += 2u;
            wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + 0x00040000u);
        }
        r->d[0] = last_d0;
        r->d[1] = 0x00040000u;
        r->a[1] = 0x00027E40u;
    }

    if ((int16_t)rd16(m, obj + 0x14u) < -0x000A)            /* 0x12251E */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + 0x00010000u); /* 0x122528 */

    if ((int16_t)rd16(m, obj + 0x14u) > -0x00A0 && rd8(m, obj + 0x22u) == 0u) { /* 0x122530..0x12253E */
        wr8(m, obj + 0x22u, 0xFFu);                         /* 0x122542 */
        Gfx_DrawCompressedTilemap(m, 0x00081778u, 0x00612000u, 0x80000000u); /* 0x122556 */
        Gfx_DrawCompressedTilemap(m, 0x00081778u, 0x00612A00u, 0x00000000u); /* 0x122570 */
    }

    if ((int16_t)rd16(m, obj + 0x3Eu) < 6)                  /* 0x12257A */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x00001000u); /* 0x122584 */

    if ((int16_t)rd16(m, obj + 0x3Au) < 4) {                /* 0x12258C */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + 0x00000200u); /* 0x122596 */
        wr32(m, obj + 0x00u, 0x00122384u);                  /* 0x12259E -> 0x12237C */
        return 0;
    }

    wr16(m, obj + 0x06u, 0x00B4u);                          /* 0x1225A2 */
    wr32(m, obj + 0x3Au, 0u);                               /* 0x1225A8 */
    wr32(m, obj + 0x00u, 0x001225B4u);                      /* 0x1225AC */
    return 0;
}
