/* ROM 0x120DF6: homing/tracking enemy init (A0 = object, +0x0C = packed arg).
 * Sets hitboxes (+0x2C..+0x32), clears +0x1C/+0x1E, seeds offsets (-0xE, -0xC)
 * and inherits parent position/angle via 0x0CCC08, sets HP (+0x24 = 0x140000),
 * score (+0x4C = 0xC8), timer (+0x06 = 0x258), speed (+0x42 = 4), horizontal flip
 * (+0x1C bit 0 from angle - 0x40 <= 0), and arms 0x120E72. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CCC08: unpack +0x0C byte offsets into +0x50/+0x52 and angle into +0x20,
 * copy parent (+0x08) position +0x14/+0x18 and add offsets. */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_out, uint32_t *d1_out, uint32_t *a1_out)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);                          /* 0x0CCC08 */
    int32_t dx = (int32_t)(int8_t)(uint8_t)(d0 >> 24);           /* 0x0CCC0C bfexts {0:8} */
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)dx)); /* 0x0CCC10 */
    int32_t dy = (int32_t)(int8_t)(uint8_t)(d0 >> 16);           /* 0x0CCC14 bfexts {8:8} */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)dy)); /* 0x0CCC18 */
    uint32_t d1 = (d0 >> 8) & 0xFFu;                             /* 0x0CCC1C bfextu {16:8} */
    wr16(m, obj + 0x20u, (uint16_t)d1);                          /* 0x0CCC20 */

    uint32_t a1 = rd32(m, obj + 0x08u);                          /* 0x0CCC24 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));                   /* 0x0CCC28 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));                   /* 0x0CCC2E */
    wr16(m, obj + 0x34u, 0u);                                    /* 0x0CCC34 */
    wr32(m, obj + 0x28u, 0x00010000u);                           /* 0x0CCC38 */

    d0 = setw(d0, rd16(m, obj + 0x50u));                         /* 0x0CCC40 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x0CCC44 */
    d0 = setw(d0, rd16(m, obj + 0x52u));                         /* 0x0CCC48 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x0CCC4C */
    wr16(m, obj + 0x3Au, 0u);                                    /* 0x0CCC50 */
    wr16(m, obj + 0x3Eu, 0u);                                    /* 0x0CCC54 */

    *d0_out = d0;
    *d1_out = d1;
    *a1_out = a1;
}

int Fn_120DF6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0, d1, a1;

    wr16(m, obj + 0x2Cu, 6u);                                    /* 0x120DF6 */
    wr16(m, obj + 0x2Eu, 0x0017u);                               /* 0x120DFC */
    wr16(m, obj + 0x30u, 3u);                                    /* 0x120E02 */
    wr16(m, obj + 0x32u, 0x0014u);                               /* 0x120E08 */
    wr16(m, obj + 0x1Eu, 0u);                                    /* 0x120E0E */
    wr16(m, obj + 0x1Cu, 0u);                                    /* 0x120E12 */
    wr16(m, obj + 0x50u, 0xFFF2u);                               /* 0x120E16 */
    wr16(m, obj + 0x52u, 0xFFF4u);                               /* 0x120E1C */

    sub_0CCC08(m, obj, &d0, &d1, &a1);                          /* 0x120E22 */

    wr32(m, obj + 0x28u, 0x27100000u);                           /* 0x120E28 */
    wr32(m, obj + 0x24u, 0x00140000u);                           /* 0x120E30 */
    wr16(m, obj + 0x4Cu, 0x00C8u);                               /* 0x120E38 */
    wr8(m, obj + 0x4Fu, 0u);                                     /* 0x120E3E */
    wr16(m, obj + 0x3Eu, 0u);                                    /* 0x120E42 */
    wr16(m, obj + 0x3Au, 0u);                                    /* 0x120E46 */

    d0 = setw(d0, rd16(m, obj + 0x20u));                         /* 0x120E4A */
    uint8_t b = (uint8_t)((uint8_t)d0 - 0x40u);                  /* 0x120E4E */
    d0 = (d0 & 0xFFFFFF00u) | b;
    wr8(m, obj + 0x1Cu, ((int8_t)b <= 0) ? 1u : 0u);             /* 0x120E54..0x120E58 */

    wr16(m, obj + 0x06u, 0x0258u);                               /* 0x120E5E */
    wr16(m, obj + 0x42u, 4u);                                    /* 0x120E64 */
    wr32(m, obj + 0x00u, 0x00120E72u);                           /* 0x120E6A */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
