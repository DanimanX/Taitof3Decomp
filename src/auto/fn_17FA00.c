/* ROM 0x17FA00: spawn/bounds-check init for boss sub-part (A0 = object).
 * Copies A5+0x1D28 to +0x4C (freeing self if 0), inherits offset position from
 * parent via 0x17BD68, sets +0x20 = (+0x0F << 8), frees self if +0x50/+0x54
 * outside [-16..335] x [-16..247] (cmp2.w 0x1849F8/0x1849FC), otherwise spawns
 * 0x17C856 (group 11, count 1, arg 0), sets +0x06 = 1, and arms 0x17FA66. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x17BD68 with D0/D1 register tracking. */
static void sub_17BD68(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int32_t dx = (int32_t)(int8_t)rd8(m, obj + 0x0Cu);
    int32_t dy = (int32_t)(int8_t)rd8(m, obj + 0x0Du);
    wr16(m, obj + 0x16u, (uint16_t)dx);
    wr16(m, obj + 0x1Au, (uint16_t)dy);

    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);

    r->d[0] = setw((uint32_t)dx, x);
    r->d[1] = setw((uint32_t)dy, y);
}

int Fn_17FA00(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);                /* 0x17FA00 */
    wr32(m, obj + 0x4Cu, tgt);
    if (tgt == 0u) {                                        /* 0x17FA06 */
        Obj_Free(m, obj);                                   /* 0x17C78E */
        return 0;
    }

    sub_17BD68(m, r);                                       /* 0x17FA0A */
    uint32_t d0 = ((uint32_t)rd8(m, obj + 0x0Fu) << 8);     /* 0x17FA0E..0x17FA14 */
    wr16(m, obj + 0x20u, (uint16_t)d0);                     /* 0x17FA16 */

    uint16_t x = rd16(m, obj + 0x50u);                      /* 0x17FA1A */
    d0 = setw(d0, x);
    uint16_t x_lo = rd16(m, 0x001849F8u);
    uint16_t x_hi = rd16(m, 0x001849FAu);
    if ((uint16_t)(x - x_lo) > (uint16_t)(x_hi - x_lo)) {   /* 0x17FA1E..0x17FA26 */
        Obj_Free(m, obj);
        r->d[0] = d0;
        return 0;
    }

    uint16_t y = rd16(m, obj + 0x54u);                      /* 0x17FA2A */
    d0 = setw(d0, y);
    uint16_t y_lo = rd16(m, 0x001849FCu);
    uint16_t y_hi = rd16(m, 0x001849FEu);
    if ((uint16_t)(y - y_lo) > (uint16_t)(y_hi - y_lo)) {   /* 0x17FA2E..0x17FA36 */
        Obj_Free(m, obj);
        r->d[0] = d0;
        return 0;
    }

    r->d[0] = Obj_Spawn(m, obj, 0x0017C856u, 0x000Bu, 1u, 0u); /* 0x17FA3A..0x17FA52 */
    wr16(m, obj + 0x06u, 1u);                               /* 0x17FA56 */
    wr32(m, obj, 0x0017FA66u);                              /* 0x17FA5E */
    return 0;
}
