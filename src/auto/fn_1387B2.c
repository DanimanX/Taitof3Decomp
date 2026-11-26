/* ROM 0x1387B2: multi-part boss component initializer (A0 = object).
 * Resets common enemy fields (0x11B2F6), loads +0x50/+0x52/+0x2C..+0x32 from
 * descriptor +0x0C via 0x11B31C, reads HP (+0x24.w) and score (+0x4C.w), stores
 * updated descriptor back to +0x0C, sets bits 4 and 2 of +0x34.b, clears
 * +0x22.w, sets +0x59 = 0xFF, latches +0x44 = +0x24, and installs 0x1387F0. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x11B31C with D0w (last sign-extended hitbox byte) and A1 register tracking. */
static void sub_11B31C_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io, uint32_t *d0_io)
{
    uint32_t a1 = *a1_io;
    wr16(m, obj + 0x50u, rd16(m, a1)); a1 += 2u;          /* 0x11B31C */
    wr16(m, obj + 0x52u, rd16(m, a1)); a1 += 2u;          /* 0x11B320 */
    wr16(m, obj + 0x54u, 0u);                             /* 0x11B324 */
    wr16(m, obj + 0x56u, 0u);                             /* 0x11B328 */
    uint16_t d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x2Cu, d0w);                            /* 0x11B32C..0x11B330 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x30u, d0w);                            /* 0x11B334..0x11B338 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x2Eu, d0w);                            /* 0x11B33C..0x11B340 */
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x32u, d0w);                            /* 0x11B344..0x11B348 */
    *a1_io = a1;
    *d0_io = setw(*d0_io, d0w);
}

int Fn_1387B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];

    Obj_11B2F6(m, obj);                                   /* 0x1387B2 */
    uint32_t a1 = rd32(m, obj + 0x0Cu);                   /* 0x1387B8 */
    sub_11B31C_regs(m, obj, &a1, &d0);                    /* 0x1387BC */
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;          /* 0x1387C2 */
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;          /* 0x1387C6 */
    wr32(m, obj + 0x0Cu, a1);                             /* 0x1387CA */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x1387CE */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* 0x1387D4 */
    wr16(m, obj + 0x22u, 0u);                             /* 0x1387DA */
    wr8(m, obj + 0x59u, 0u);                              /* 0x1387DE: sf */
    wr32(m, obj + 0x44u, rd32(m, obj + 0x24u));           /* 0x1387E2 */
    wr32(m, obj, 0x001387F0u);                            /* 0x1387E8 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
