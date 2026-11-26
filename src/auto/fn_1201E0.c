/* ROM 0x1201E0: select pose record from (+0x0C) via A5+0x3262 (or animate
 * (+0x0C)+0x10 via 0x0CBD52 when A5+0x3262 == 0) and unpack into A0 via
 * 0x12048C. Leaves D0.l = sign-extended last byte, D1.w updated when
 * 0x0CBD52 runs, and A1 advanced by 10. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x12048C: unpack offset, sprite ptr, and hitbox bytes from A1. */
static uint32_t sub_12048C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_io)
{
    uint32_t d0 = *d0_io;
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x50u, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x52u, (uint16_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Cu, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)d0);
    d0 = setw(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x32u, (uint16_t)d0);
    *d0_io = d0;
    return a1;
}

int Fn_1201E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x0Cu);                      /* 0x1201E0 */
    uint32_t d0 = (uint32_t)rd8(m, F3_A5 + 0x3262u);        /* 0x1201E4..0x1201E6 */

    if (d0 != 0u) {                                          /* 0x1201EA */
        a1 = rd32(m, a1 + (uint32_t)((int32_t)(int16_t)d0 * 4 - 4)); /* 0x1201EE */
    } else {
        a1 += 0x10u;                                         /* 0x1201F6 */
        Obj_Animate(m, obj, a1);                             /* 0x1201FA -> 0x0CBD52 */
        d0 = setw(d0, rd16(m, obj + 0x3Au));
        r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));
        a1 = rd32(m, obj + 0x10u);                           /* 0x120200 */
    }

    r->a[1] = sub_12048C(m, obj, a1, &d0);                   /* 0x120204 -> 0x12048C */
    r->d[0] = d0;
    return 0;
}
