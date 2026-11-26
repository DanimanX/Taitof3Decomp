/* ROM 0x130EE2: homing missile/projectile initializer (A0 = object).
 * Resets common fields via 0x0CBD86, configures hitboxes (+0x2C..+0x32),
 * HP/damage (+0x24/+0x28), speed (+0x42 = 0x14), palette (+0x1D = 0x1A),
 * inherits parent (+0x08) position (+0x14/+0x18) adjusted by signed bytes
 * (+0x0C[0]-0x10, +0x0C[1]-0x12), sets angle (+0x20.w = arg<<8? sf D0b),
 * sets timer +0x06 = 0x20, and arms tick 0x130F58. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_130EE2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);                             /* 0x130EE2 -> 0x0CBD86 */
    wr16(m, obj + 0x2Cu, 0x0005u);                       /* 0x130EE8 */
    wr16(m, obj + 0x30u, 0x0005u);                       /* 0x130EEE */
    wr16(m, obj + 0x2Eu, 0x001Au);                       /* 0x130EF4 */
    wr16(m, obj + 0x32u, 0x001Au);                       /* 0x130EFA */
    wr16(m, obj + 0x24u, 0x0001u);                       /* 0x130F00 */
    wr16(m, obj + 0x28u, 0x0001u);                       /* 0x130F06 */
    wr16(m, obj + 0x42u, 0x0014u);                       /* 0x130F0C */
    wr8(m, obj + 0x1Du, 0x1Au);                          /* 0x130F12 */

    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x130F18 */
    r->a[1] = a1;
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));           /* 0x130F1C */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));           /* 0x130F22 */

    uint32_t d0 = rd32(m, obj + 0x0Cu);                  /* 0x130F28 */
    uint32_t d1 = (uint32_t)(int32_t)(int8_t)(d0 >> 24); /* 0x130F2C bfexts */
    d1 = setw(d1, (uint16_t)((uint16_t)d1 - 0x0010u));   /* 0x130F30 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d1)); /* 0x130F34 */

    d1 = (uint32_t)(int32_t)(int8_t)(d0 >> 16);          /* 0x130F38 bfexts */
    d1 = setw(d1, (uint16_t)((uint16_t)d1 - 0x0012u));   /* 0x130F3C */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1)); /* 0x130F40 */

    d0 &= 0xFFFFFF00u;                                   /* 0x130F44 sf D0b */
    wr16(m, obj + 0x20u, (uint16_t)d0);                  /* 0x130F46 */
    wr16(m, obj + 0x06u, 0x0020u);                       /* 0x130F4A */
    wr32(m, obj + 0x00u, 0x00130F58u);                   /* 0x130F50 */

    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
