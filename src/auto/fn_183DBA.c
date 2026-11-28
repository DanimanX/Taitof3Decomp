/* ROM 0x183DBA: boss sub-part init (A0 = object).
 * Copies A5+0x1D28 into +0x4C (a long <= 0 frees the object through 0x17C78E,
 * i.e. 0x0105EC), inherits the parent-relative position via 0x17BD68, then
 * frees the object again when +0x50 or +0x54 falls outside the cmp2.w bounds
 * [-16..335] x [-16..247] (0x1849F8 / 0x1849FC). In range it copies +0x0E to
 * +0x64 and arms 0x183DF8. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x17BD68 with D0/D1 register tracking: +0x16/+0x1A = signed bytes at
 * +0x0C/+0x0D, position = parent (+0x08) +0x50/+0x54 offset by those, mirrored
 * into +0x14/+0x18. D0/D1 keep the sign-extended offsets in their high words. */
static void sub_17BD68(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int32_t dx = (int32_t)(int8_t)rd8(m, obj + 0x0Cu);        /* 0x17BD68 bfexts */
    int32_t dy = (int32_t)(int8_t)rd8(m, obj + 0x0Du);        /* 0x17BD6E bfexts */
    wr16(m, obj + 0x16u, (uint16_t)dx);                       /* 0x17BD74 */
    wr16(m, obj + 0x1Au, (uint16_t)dy);                       /* 0x17BD78 */

    uint32_t a4 = rd32(m, obj + 0x08u);                       /* 0x17BD7C */
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);                                  /* 0x17BD90 */
    wr16(m, obj + 0x54u, y);                                  /* 0x17BD94 */
    wr16(m, obj + 0x14u, x);                                  /* 0x17BD98 */
    wr16(m, obj + 0x18u, y);                                  /* 0x17BD9C */

    r->d[0] = setw((uint32_t)dx, x);
    r->d[1] = setw((uint32_t)dy, y);
    r->a[4] = a4;
}

/* cmp2.w at 0x183DCE / 0x183DDE: C is set when the value leaves the inclusive
 * [lo..hi] range. The bounds here wrap around zero (0xFFF0..0x014F), so the
 * signed range test is the unsigned difference one. */
static int outside_bounds(const f3mem_t *m, uint16_t v, uint32_t bounds)
{
    uint16_t lo = rd16(m, bounds);
    uint16_t hi = rd16(m, bounds + 2u);
    return (uint16_t)(v - lo) > (uint16_t)(hi - lo);
}

int Fn_183DBA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t owner = rd32(m, F3_A5 + 0x1D28u);                /* 0x183DBA */
    wr32(m, obj + 0x4Cu, owner);
    if ((int32_t)owner <= 0) {                                /* 0x183DC0 */
        Obj_Free(m, obj);                                     /* 0x17C78E */
        return 0;
    }

    sub_17BD68(m, r);                                         /* 0x183DC4 */

    uint16_t x = rd16(m, obj + 0x50u);                        /* 0x183DCA */
    r->d[0] = setw(r->d[0], x);
    if (outside_bounds(m, x, 0x001849F8u)) {                  /* 0x183DCE */
        Obj_Free(m, obj);
        return 0;
    }

    uint16_t y = rd16(m, obj + 0x54u);                        /* 0x183DDA */
    r->d[0] = setw(r->d[0], y);
    if (outside_bounds(m, y, 0x001849FCu)) {                  /* 0x183DDE */
        Obj_Free(m, obj);
        return 0;
    }

    wr16(m, obj + 0x64u, rd16(m, obj + 0x0Eu));               /* 0x183DEA */
    wr32(m, obj, 0x00183DF8u);                                /* 0x183DF0 */
    return 0;
}
