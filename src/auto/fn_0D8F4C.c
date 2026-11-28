#include "fn_auto.h"
/* ROM 0x0D8F4C: place an object from its parent's position (A0 = object,
 * A1 = (A0+0x08) parent). Clears the velocity/animation fields (+0x1C, +0x1E,
 * +0x34, +0x3A, +0x3E, +0x24, +0x28) through 0x0CBD86, copies the parent's
 * +0x14/+0x18 and shifts them by the signed {dx.b, dy.b} at +0x0C minus 0x10,
 * takes the {kind.b} at +0x0E into +0x20, clears +0x42 and arms 0x0D8F9A. */
int Fn_0D8F4C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0D8F4C jsr 0x0CBD86 */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);

    r->a[1] = rd32(m, obj + 0x08u);          /* 0x0D8F52 movea.l (8,A0),A1 */
    wr32(m, obj + 0x14u, rd32(m, r->a[1] + 0x14u)); /* 0x0D8F56 */
    wr32(m, obj + 0x18u, rd32(m, r->a[1] + 0x18u)); /* 0x0D8F5C */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u)); /* 0x0D8F62 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u)); /* 0x0D8F68 */

    uint32_t d = rd32(m, obj + 0x0Cu);       /* {dx.b, dy.b, kind.b, pad.b} */
    r->d[0] = (uint32_t)(int32_t)(int8_t)(d >> 24);      /* 0x0D8F6E bfexts {0:8} */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)r->d[0])); /* 0x0D8F74 */
    r->d[0] = (uint32_t)(int32_t)(int8_t)((d >> 16) & 0xFFu); /* 0x0D8F78 bfexts {8:8} */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)r->d[0])); /* 0x0D8F7E */
    r->d[0] = (d >> 8) & 0xFFu;              /* 0x0D8F82 bfextu {16:8} */
    wr16(m, obj + 0x20u, (uint16_t)r->d[0]); /* 0x0D8F88 */
    wr16(m, obj + 0x42u, 0u);                /* 0x0D8F8C */
    wr32(m, obj, 0x000D8F9Au);               /* 0x0D8F92 */
    return 0;                                /* 0x0D8F98 rts */
}
