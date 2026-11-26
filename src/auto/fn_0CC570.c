/* ROM 0x0CC570: child projectile/sub-part initializer (A0 = object).
 * Sets hitbox dimensions (+0x2C..+0x32), palette/timer (+0x22, +0x50, +0x52),
 * unpacks +0x0C via 0x0CCC08 into +0x50/+0x52/+0x20 and parent +0x14/+0x18,
 * sets HP=1, +0x4C=10, state=0x0258, and arms 0x0CC5CA. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CCC08: unpack +0x0C signed/unsigned bytes and inherit parent pos. */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t arg = rd32(m, obj + 0x0Cu);
    int16_t dx = (int16_t)(int8_t)(arg >> 24);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)dx));

    int16_t dy = (int16_t)(int8_t)(arg >> 16);
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)dy));

    uint16_t d1w = (uint16_t)(uint8_t)(arg >> 8);
    *d1_io = (uint32_t)d1w;
    wr16(m, obj + 0x20u, d1w);

    uint32_t a1 = rd32(m, obj + 0x08u);
    *a1_io = a1;
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);

    uint16_t ox = rd16(m, obj + 0x50u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + ox));
    uint16_t oy = rd16(m, obj + 0x52u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + oy));
    *d0_io = setw(arg, oy);

    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
}

int Fn_0CC570(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    wr16(m, obj + 0x2Cu, 1u);                           /* 0x0CC570 */
    wr16(m, obj + 0x2Eu, 0x000Eu);                      /* 0x0CC576 */
    wr16(m, obj + 0x30u, 1u);                           /* 0x0CC57C */
    wr16(m, obj + 0x32u, 0x000Eu);                      /* 0x0CC582 */
    wr16(m, obj + 0x1Eu, 0u);                           /* 0x0CC588 */
    wr16(m, obj + 0x1Cu, 0u);                           /* 0x0CC58C */
    wr16(m, obj + 0x22u, 0x0A00u);                      /* 0x0CC590 */
    wr16(m, obj + 0x42u, 0u);                           /* 0x0CC596 */
    wr16(m, obj + 0x50u, 0xFFF8u);                      /* 0x0CC59A */
    wr16(m, obj + 0x52u, 0xFFF8u);                      /* 0x0CC5A0 */

    sub_0CCC08(m, obj, &d0, &d1, &a1);                  /* 0x0CC5A6 */

    wr32(m, obj + 0x24u, 1u);                           /* 0x0CC5AA */
    wr16(m, obj + 0x4Cu, 0x000Au);                      /* 0x0CC5B2 */
    wr8(m, obj + 0x4Fu, 0u);                            /* 0x0CC5B8 */
    wr16(m, obj + 0x06u, 0x0258u);                      /* 0x0CC5BC */
    wr32(m, obj, 0x000CC5CAu);                          /* 0x0CC5C2 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
