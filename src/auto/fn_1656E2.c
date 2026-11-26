/* ROM 0x1656E2: stage warp/background fade and transition tick (A0 = object).
 * Computes fade level from |A5-0x5DA0 - 0x2C0| / 0x58 and fills line-fade
 * RAM 0x626200 + (-0x5D56,A5) across 0xE8 scanlines. When A5-0x5DA0 >= 0x2C0
 * and A5+0x41EC < 0x2C0, initializes warp background via 0x165C28 (clears
 * PF1/line RAM, fills 0x62A400/0x624400, draws 8 compressed tilemaps, updates
 * A5+0x4234..+0x4240) and spawns 0x1659F0 into A5+0x3BBA. While A5-0x5DA0
 * <= 0x580, re-arms 0x1656E2; otherwise clears PF0b (0x005918), fills
 * 0x628200/0x62B400, sets +0x44 = 7 and +0x06 = 8, and transitions to
 * 0x1657EE. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x176198: fill 0xE8 words (0x1D0 bytes) at a0 + (-0x5D56,A5) with val */
static void sub_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0u; off < 0x01D0u; off += 2u)
        wr16(m, a0 + off, val);
}

/* ROM 0x1768CA: quadratic line-table ramp in bits 0..8 of a0 + (-0x5D56,A5) */
static void sub_1768CA(const f3mem_t *m, uint32_t base, int16_t d7w, int16_t d6w,
                       uint32_t init, uint32_t d3, uint32_t d4)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    wr32(m, F3_A5 + 0x4298u, init);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {
        wr32(m, F3_A5 + 0x4298u, (uint32_t)(-(int32_t)rd32(m, F3_A5 + 0x4298u)));
        d3 = (uint32_t)(-(int32_t)d3);
        d4 = (uint32_t)(-(int32_t)d4);
    }
    int16_t d5w = (d7w <= d6w) ? 1 : -1;
    for (;;) {
        uint16_t d2w = rd16(m, F3_A5 + 0x4298u);
        uint32_t addr = a0 + (uint32_t)(int32_t)d7w * 2u;
        uint16_t d0w = rd16(m, addr);
        d0w = (uint16_t)((d0w & 0xFE00u) | (d2w & 0x01FFu));
        wr16(m, addr, d0w);
        d3 += d4;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d3);
        d7w = (int16_t)(d7w + d5w);
        if (d5w < 0) {
            if (d6w > d7w)
                break;
        } else {
            if (d7w > d6w)
                break;
        }
    }
}

/* ROM 0x165C28: setup stage-clear warp playfield and line RAM; returns final A0 */
static uint32_t sub_165C28(const f3mem_t *m)
{
    static const struct { uint32_t src; uint32_t dst; } maps[8] = {
        { 0x0009018Eu, 0x00613000u },
        { 0x00090208u, 0x00613400u },
        { 0x0009023Au, 0x00613020u },
        { 0x000902AEu, 0x00613420u },
        { 0x0009018Eu, 0x00613040u },
        { 0x00090208u, 0x00613440u },
        { 0x0009023Au, 0x00613060u },
        { 0x000902AEu, 0x00613460u },
    };
    Gfx_ClearPf1AndLineRam(m);                           /* 0x165C28 */
    sub_176198(m, 0x0062A400u, 0x0000u);                 /* 0x165C2E..0x165C3E */
    sub_1768CA(m, 0x00624400u, 0, 0x00E7, 0u, 0u, 0u);   /* 0x165C42..0x165C68 */
    for (int i = 0; i < 8; i++)                          /* 0x165C6C..0x165D38 */
        Gfx_DrawCompressedTilemap(m, maps[i].src, maps[i].dst, 0u);
    Obj_16AB26(m, F3_A5 + 0x4234u, 0x00010000u, 0u);     /* 0x165D3C..0x165D50 */
    Obj_16AB26(m, F3_A5 + 0x423Cu, 0x00001000u, 0u);     /* 0x165D56..0x165D6A */
    return F3_A5 + 0x4240u;
}

int Fn_1656E2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = setw(r->d[0], 4u);                     /* 0x1656E2 */
    uint32_t d1 = 0u;                                    /* 0x1656E6 */
    uint16_t d1w = (uint16_t)(rd16(m, F3_A5 - 0x5DA0u) - 0x02C0u); /* 0x1656E8..0x1656EC */
    if ((int16_t)d1w <= 0)                               /* 0x1656F0 */
        d1w = (uint16_t)(-(int16_t)d1w);                 /* 0x1656F4 */
    d1 = d1w;
    {
        uint16_t q = (uint16_t)(d1 / 0x0058u);           /* 0x1656F6 divu.w */
        uint16_t rem = (uint16_t)(d1 % 0x0058u);
        d1 = ((uint32_t)rem << 16) | q;
    }

    {
        int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u);
        uint32_t lram = 0x00626200u + (uint32_t)(int32_t)off; /* 0x1656FE..0x165704 */
        uint16_t c0w = (uint16_t)(d0 & 0x000Fu);         /* 0x165708 */
        if ((int16_t)c0w > 8)                            /* 0x16570C..0x165714 */
            c0w = 8u;
        uint16_t c1w = (uint16_t)(d1 & 0x000Fu);         /* 0x165718 */
        if ((int16_t)c1w > 8)                            /* 0x16571C..0x165724 */
            c1w = 8u;
        uint16_t hi = (uint16_t)(((c0w << 8) + (uint16_t)(8u - c0w)) << 4);
        uint16_t lo = (uint16_t)((c1w << 8) + (uint16_t)(8u - c1w));
        uint16_t fill = (uint16_t)~(hi | lo);            /* 0x16573E..0x165740 */
        for (int16_t d7 = 0x00E7; d7 >= 0; d7--)         /* 0x165742..0x16574E */
            wr16(m, lram + (uint32_t)d7 * 2u, fill);
    }

    if ((int16_t)rd16(m, F3_A5 - 0x5DA0u) >= 0x02C0 &&   /* 0x165756..0x16575C */
        (int16_t)rd16(m, F3_A5 + 0x41ECu) < 0x02C0) {    /* 0x165760..0x165766 */
        uint32_t parent_a0 = sub_165C28(m);              /* 0x16576E */
        d1 = 0u;
        d0 = Obj_Spawn(m, parent_a0, 0x001659F0u, 0u, 1u, 0u); /* 0x165772..0x16578C */
        wr32(m, F3_A5 + 0x3BBAu, d0);                    /* 0x165790 */
    }

    if ((int16_t)rd16(m, F3_A5 - 0x5DA0u) <= 0x0580) {   /* 0x165798..0x16579E */
        wr32(m, obj + 0x00u, 0x001656E2u);               /* 0x1656DA */
        r->d[0] = d0;
        r->d[1] = d1;
        return 0;                                        /* 0x1656E0 */
    }

    Gfx_ClearPf0b(m);                                    /* 0x1657A6 */
    sub_176198(m, 0x00628200u, 0x0080u);                 /* 0x1657AC..0x1657BC */
    sub_176198(m, 0x0062B400u, 0x7008u);                 /* 0x1657C0..0x1657D0 */
    wr16(m, obj + 0x44u, 7u);                            /* 0x1657D8 */
    wr16(m, obj + 0x06u, 8u);                            /* 0x1657DE (0x1657E4 beq folded) */
    wr32(m, obj + 0x00u, 0x001657EEu);                   /* 0x1657E6 */
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;                                            /* 0x1657EC */
}
