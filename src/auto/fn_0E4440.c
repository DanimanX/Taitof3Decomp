/* ROM 0x0E4440: trail/afterimage node initializer (A0 = object).
 * 0x0CEDE4 seeds the mover from 0x0DC84E and 0x0D0678 the animation from
 * 0x0E356A; the angle word +0x1E and the position (+0x14, +0x18) are copied
 * from the grandparent ((+0x08)->+0x08). The angle byte then scales two polar
 * offsets through 0x0D4A2C (D0 = 0x0E, D1 = 0x16) added to +0x14/+0x18, and
 * 0x0D4A12(2, angle) after a swap becomes the step velocity at +0x0C.
 * +0x22 is set and the tick re-arms at 0x0E44A6. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"        /* Obj_InitMoverB, Obj_SetAnim */
#include "../objects/obj_shared_ext.h" /* Obj_0CEDE4 */

/* ROM 0x0D4A0C: the 0x10000-0x101*i angle scale table at 0x0D4A5A and the
 * 0x0D4E5A dispatch table of "multiply D0 by n" routines implement
 * 0x0D4A12 / 0x0D4A2C: D0 = swap(scale * d0), D1 = swap(scale * d1). */

/* ROM 0x0D4A12: D0 = swap(table[d1w] * d0w), D1 = d0w. D2-D4 preserved. */
static uint32_t sub_0D4A12(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint32_t *out_d1)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)(int16_t)d1w * 4u);
    uint32_t prod = v * (uint32_t)d0w;
    *out_d1 = (uint32_t)d0w;
    return (prod << 16) | (prod >> 16);
}

/* ROM 0x0D4A2C: D0 = swap(table[d2w] * d0w), D1 = swap(table[d2w] * d1w). */
static void sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)(int16_t)d2w * 4u);
    uint32_t p1 = v * (uint32_t)d1w;
    uint32_t p0 = v * (uint32_t)d0w;
    *out_d1 = (p1 << 16) | (p1 >> 16);
    *out_d0 = (p0 << 16) | (p0 >> 16);
}

static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
}

int Fn_0E4440(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000DC84Eu);                    /* 0x0E4440..0x0E4444 */
    Obj_SetAnim(m, obj, 0x000E356Au);                   /* 0x0E444A..0x0E444E */

    uint32_t gp = rd32(m, rd32(m, obj + 0x08u) + 0x08u); /* 0x0E4454..0x0E4458 */
    wr16(m, obj + 0x1Eu, rd16(m, gp + 0x1Eu));          /* 0x0E445C */
    wr32(m, obj + 0x14u, rd32(m, gp + 0x14u));          /* 0x0E4462 */
    wr32(m, obj + 0x18u, rd32(m, gp + 0x18u));          /* 0x0E4468 */

    uint16_t ang = (uint16_t)rd8(m, obj + 0x1Eu);       /* 0x0E4472..0x0E4474 */
    uint32_t d0, d1;
    sub_0D4A2C(m, 0x000Eu, 0x0016u, ang, &d0, &d1);     /* 0x0E446E..0x0E447A */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x0E4480 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1)); /* 0x0E4484 */

    d0 = sub_0D4A12(m, 0x0002u, ang, &d1);              /* 0x0E4488..0x0E448C */
    d0 = swap32(d0);                                    /* 0x0E4492 swap D0 */
    wr32(m, obj + 0x0Cu, d0);                           /* 0x0E4494 */
    wr8(m, obj + 0x22u, 1u);                            /* 0x0E4498 */
    wr32(m, obj + 0x00u, 0x000E44A6u);                  /* 0x0E449E */

    r->a[1] = gp;
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
