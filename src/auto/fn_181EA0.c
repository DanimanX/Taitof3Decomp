/* ROM 0x181EA0: spawn initial pair of 0x181FA8 child objects (A0 = object).
 * Inherits offset position from parent via 0x17BD68, spawns two 0x181FA8
 * objects in group 9..10 with args 0xFFB40010 and 0x004C0010, sets +0x06 = 5,
 * and arms next tick 0x181EF0. */
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

int Fn_181EA0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_17BD68(m, r);                                   /* 0x181EA0 */
    (void)Obj_Spawn(m, obj, 0x00181FA8u, 9u, 2u, 0xFFB40010u); /* 0x181EA4..0x181EBE */
    r->d[0] = Obj_Spawn(m, obj, 0x00181FA8u, 9u, 2u, 0x004C0010u); /* 0x181EC2..0x181EDC */
    wr16(m, obj + 0x06u, 5u);                           /* 0x181EE0 */
    wr32(m, obj, 0x00181EF0u);                          /* 0x181EE8 */
    return 0;                                           /* 0x181EEE */
}
