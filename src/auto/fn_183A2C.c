#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"

/* ROM 0x183A2C: spawn-wait tail of the 0x183B1E family (A0 = object).
 *
 * +0x06 is a countdown: while it stays positive the routine re-arms 0x183A2C
 * at +0x00 (0x183A24 thunk). Once it expires the object is freed through
 * 0x17C78E when +0x50/+0x54 fall outside the on-screen box (cmp2.w pairs at
 * ROM 0x1849F8 / 0x1849FC, [-0x10..0x14F] x [-0x10..0xF7]). Otherwise the
 * stats come from the 0x183B64 record via 0x184888, animation index 0 (word
 * at 0x183B7A = 0, script body 0x183B7C) is initialized through the 0x179A5C
 * table, script 0x183B94 is installed, +0x20 gets a 0x1794A4 random byte
 * (+0xFF range), +0x64 = 0x40, callback +0x0C = 0x183AB0 and the tick becomes
 * the 0x17C14C step (0x17C144 thunk). */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x1794A4: D0w = LCG value masked by the bit length of d0, negated
 * according to bit 24 of the state; the shared LCG at A5+0x4B3C advances by
 * state = swap(state * 5 - 1). */
static uint16_t sub_1794A4(const f3mem_t *m, uint16_t d0)
{
    uint16_t r = d0;
    if (d0 != 0u) {
        int off = 0;
        while (off < 32 && !((uint32_t)d0 & (0x80000000u >> off)))
            off++;
        int16_t d1w = (int16_t)(off - 16);
        uint16_t d2w = rd16(m, 0x179530u + (uint32_t)((int32_t)d1w * 2));
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x4B3Cu) & d2w);
        if ((int16_t)r < (int16_t)v)
            v = (uint16_t)(v - r);
        r = v;
        if (rd8(m, F3_A5 + 0x4B3Cu) & 1u)
            r = (uint16_t)(0u - r);
    }
    uint32_t s = rd32(m, F3_A5 + 0x4B3Cu) * 5u - 1u;
    wr32(m, F3_A5 + 0x4B3Cu, (s << 16) | (s >> 16));
    return r;
}

/* ROM 0x184888: seed the object's stats from the 22-byte record at A1; the
 * +0x24 word (record +0x04) is divided by 10 into the long at +0x24. */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1 + 0x00u));
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x02u));
    wr16(m, obj + 0x24u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x28u, rd16(m, a1 + 0x06u));
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 0x08u));
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x0Cu));
    wr16(m, obj + 0x62u, rd16(m, a1 + 0x10u));
    wr32(m, obj + 0x66u, rd32(m, a1 + 0x12u));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);   /* divul.l #0xA,D0:D0 */
}

/* cmp2.w (base)/(base+2),D0w + bcs: the word is outside the signed inclusive
 * (lower, upper) pair. */
static int outside(const f3mem_t *m, uint32_t base, uint16_t v)
{
    uint16_t lo = rd16(m, base);
    uint16_t hi = rd16(m, base + 2u);
    return (uint16_t)(v - lo) > (uint16_t)(hi - lo);
}

int Fn_183A2C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x183A2C */
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x00183A2Cu);                        /* 0x183A24 */
        return 0;
    }

    uint16_t x = rd16(m, obj + 0x50u);                            /* 0x183A32 */
    r->d[0] = setw(r->d[0], x);
    if (outside(m, 0x001849F8u, x)) {                             /* 0x183A36 */
        Obj_Free(m, obj);                                         /* 0x17C78E */
        return 0;
    }

    uint16_t y = rd16(m, obj + 0x54u);                            /* 0x183A42 */
    r->d[0] = setw(r->d[0], y);
    if (outside(m, 0x001849FCu, y)) {                             /* 0x183A46 */
        Obj_Free(m, obj);                                         /* 0x17C78E */
        return 0;
    }

    sub_184888(m, obj, 0x00183B64u);                              /* 0x183A52 */

    {
        uint32_t a1 = 0x00183B7Au;                                /* 0x183A5E */
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)                       /* 0x183A74 */
            return -1;
    }

    r->a[1] = 0x00183B94u;                                        /* 0x183A7A */
    wr32(m, obj + 0x44u, 0x00183B94u);
    wr16(m, obj + 0x48u, rd16(m, 0x00183B98u));
    wr16(m, obj + 0x4Au, 0u);

    r->d[0] = 0x000000FFu;                                       /* 0x183A8E */
    {
        uint16_t v = sub_1794A4(m, 0x00FFu);
        r->d[0] = setw(r->d[0], v);
        wr8(m, obj + 0x20u, (uint8_t)v);                         /* 0x183A9A */
    }
    wr16(m, obj + 0x64u, 0x0040u);                               /* 0x183A9E */
    wr32(m, obj + 0x0Cu, 0x00183AB0u);                           /* 0x183AA4 */
    wr32(m, obj + 0x00u, 0x0017C14Cu);                           /* 0x183AAC -> 0x17C144 */
    return 0;
}
