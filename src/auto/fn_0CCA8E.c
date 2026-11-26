/* ROM 0x0CCA8E: initialize child projectile/part from +0x0C arg offsets and parent +0x08 pos. */
#include "fn_auto.h"

/* ROM 0x0CCC08: unpack +0x0C signed/unsigned bytes and inherit parent pos. */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);                 /* 0x0CCC08 */
    int32_t dx = (int32_t)d0 >> 24;                     /* 0x0CCC0C: bfexts D0{0:8},D1 */
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)dx)); /* 0x0CCC10 */

    int32_t dy = (int32_t)(int8_t)(d0 >> 16);           /* 0x0CCC14: bfexts D0{8:8},D1 */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)dy)); /* 0x0CCC18 */

    uint32_t d1 = (d0 >> 8) & 0xFFu;                    /* 0x0CCC1C: bfextu D0{16:8},D1 */
    wr16(m, obj + 0x20u, (uint16_t)d1);                 /* 0x0CCC20 */

    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x0CCC24 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));          /* 0x0CCC28 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));          /* 0x0CCC2E */
    wr16(m, obj + 0x34u, 0u);                           /* 0x0CCC34 */
    wr32(m, obj + 0x28u, 0x00010000u);                  /* 0x0CCC38 */

    uint16_t ox = rd16(m, obj + 0x50u);                 /* 0x0CCC40 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + ox)); /* 0x0CCC44 */

    uint16_t oy = rd16(m, obj + 0x52u);                 /* 0x0CCC48 */
    d0 = (d0 & 0xFFFF0000u) | oy;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + oy)); /* 0x0CCC4C */

    wr16(m, obj + 0x3Au, 0u);                           /* 0x0CCC50 */
    wr16(m, obj + 0x3Eu, 0u);                           /* 0x0CCC54 */

    *d0_io = d0;
    *d1_io = d1;
    *a1_io = a1;
}

int Fn_0CCA8E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    wr16(m, obj + 0x2Cu, 3u);                           /* 0x0CCA8E */
    wr16(m, obj + 0x2Eu, 0x0Cu);                        /* 0x0CCA94 */
    wr16(m, obj + 0x30u, 3u);                           /* 0x0CCA9A */
    wr16(m, obj + 0x32u, 0x0Cu);                        /* 0x0CCAA0 */
    wr16(m, obj + 0x1Eu, 0u);                           /* 0x0CCAA6 */
    wr16(m, obj + 0x1Cu, 0u);                           /* 0x0CCAAA */
    wr16(m, obj + 0x22u, 0x0C00u);                      /* 0x0CCAAE */
    wr16(m, obj + 0x42u, 0u);                           /* 0x0CCAB4 */
    wr16(m, obj + 0x50u, 0xFFF8u);                      /* 0x0CCAB8 */
    wr16(m, obj + 0x52u, 0xFFF8u);                      /* 0x0CCABE */

    sub_0CCC08(m, obj, &d0, &d1, &a1);                  /* 0x0CCAC4 */
    wr32(m, obj + 0x00u, 0x000CCAD0u);                  /* 0x0CCAC8 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
