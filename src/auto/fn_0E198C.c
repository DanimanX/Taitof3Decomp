/* ROM 0x0E198C: trail node initializer (A0 = object).
 * Seeds HP/hitboxes from 0x0DC84E (0x0CEDE4) and behaviour from 0x0E11A0
 * (0x0D0678). Copies +0x1E angle and (+0x3A, +0x3E) pos from grandparent
 * (+0x08)->+0x08, offsets pos by (11, 38) along angle via 0x0D4A2C, stores
 * step velocity (angle, 2) via 0x0D4A12 into +0x0C, sets +0x22 = 1, and
 * arms 0x0E19F2. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D4A12: table[d1w] * d0w, swapped. */
static uint32_t sub_0D4A12(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint32_t *out_d1)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)(int16_t)d1w * 4u);
    uint32_t prod = v * (uint32_t)d0w;
    *out_d1 = (uint32_t)d0w;
    return (prod << 16) | (prod >> 16);
}

/* ROM 0x0D4A2C: (table[d2w] * d0w swapped, table[d2w] * d1w swapped). */
static void sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)(int16_t)d2w * 4u);
    uint32_t p1 = v * (uint32_t)d1w;
    uint32_t p0 = v * (uint32_t)d0w;
    *out_d1 = (p1 << 16) | (p1 >> 16);
    *out_d0 = (p0 << 16) | (p0 >> 16);
}

int Fn_0E198C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000DC84Eu);                          /* 0x0E198C..0x0E1990 */
    Obj_StartBehaviour(m, obj, 0x000E11A0u);                  /* 0x0E1996..0x0E199A */

    uint32_t gp = rd32(m, rd32(m, obj + 0x08u) + 0x08u);      /* 0x0E19A0..0x0E19A4 */
    r->a[1] = gp;
    wr16(m, obj + 0x1Eu, rd16(m, gp + 0x1Eu));                /* 0x0E19A8 */
    wr32(m, obj + 0x14u, rd32(m, gp + 0x3Au));                /* 0x0E19AE */
    wr32(m, obj + 0x18u, rd32(m, gp + 0x3Eu));                /* 0x0E19B4 */

    uint16_t ang = (uint16_t)rd8(m, obj + 0x1Eu);             /* 0x0E19BE..0x0E19C0 */
    uint32_t d0, d1;
    sub_0D4A2C(m, 0x000Bu, 0x0026u, ang, &d0, &d1);           /* 0x0E19BA..0x0E19C6 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x0E19CC */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1)); /* 0x0E19D0 */

    d0 = sub_0D4A12(m, 0x0002u, ang, &d1);                    /* 0x0E19D4..0x0E19D8 */
    d0 = (d0 << 16) | (d0 >> 16);                             /* 0x0E19DE swap D0 */
    wr32(m, obj + 0x0Cu, d0);                                 /* 0x0E19E0 */
    wr8(m, obj + 0x22u, 1u);                                  /* 0x0E19E4 */
    wr32(m, obj, 0x000E19F2u);                                /* 0x0E19EA */
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
