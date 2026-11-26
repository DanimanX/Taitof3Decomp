/* ROM 0x196A84: boss / enemy segment initialization and 0x179A5C anim init (A0 = object).
 * Clears +0x1C/+0x1E/+0x22/+0x35, sets +0x24 = 0x00C80000 scaled twice by
 * Obj_18495E, initializes +0x28/+0x2C/+0x30 and clears +0x20, reads anim
 * index (+0x40) and pointer (+0x3A) from (+0x0C), and dispatches
 * (0x179A5C)[idx].init. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static uint32_t anim_dir_idx(const f3mem_t *m, uint32_t obj, int dirs32)
{
    uint8_t ang = rd8(m, obj + 0x20u);
    return dirs32 ? (uint32_t)((uint8_t)(ang + 4u) >> 3)
                  : (uint32_t)((uint8_t)(ang + 8u) >> 4);
}

static void anim_init_dir(const f3mem_t *m, f3regs_t *r, uint32_t obj,
                          int dirs32, uint32_t stride,
                          int has_pal, int copy_boxes, int add_xy)
{
    wr16(m, obj + 0x3Eu, 1u);
    uint32_t d0 = anim_dir_idx(m, obj, dirs32);
    uint32_t a1 = rd32(m, obj + 0x3Au) + d0 * stride;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    if (has_pal) {
        uint16_t w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w);
        d0 = setw(d0, w);
    }
    if (copy_boxes) {
        wr32(m, obj + 0x2Cu, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1));
        a1 += 4u;
    }
    if (add_xy) {
        d0 = (uint32_t)(int32_t)(int8_t)rd8(m, a1);
        uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
        r->d[1] = d1;
    }
    r->d[0] = d0;
    r->a[1] = a1;
}

static void anim_init_frame(const f3mem_t *m, f3regs_t *r, uint32_t obj,
                            int has_pal, int copy_boxes, int add_xy)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    if (has_pal) {
        uint16_t w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w);
        r->d[0] = setw(r->d[0], w);
    }
    if (copy_boxes) {
        wr32(m, obj + 0x2Cu, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1));
        a1 += 4u;
    }
    if (add_xy) {
        uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, a1);
        uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
        r->d[0] = d0;
        r->d[1] = d1;
    }
    r->a[1] = a1;
}

/* ROM 0x179A5C[idx].init dispatch with exact D0/D1/A1 register outputs. */
static int anim_init_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u:  anim_init_frame(m, r, obj, 0, 0, 0); return 0; /* 0x179550 */
    case 1u:  anim_init_dir(m, r, obj, 0,  4u, 0, 0, 0); return 0; /* 0x1795F6 */
    case 2u:  anim_init_dir(m, r, obj, 1,  4u, 0, 0, 0); return 0; /* 0x17960A */
    case 3u:  anim_init_frame(m, r, obj, 0, 1, 0); return 0; /* 0x179572 */
    case 4u:  anim_init_dir(m, r, obj, 0, 12u, 0, 1, 0); return 0; /* 0x17961E */
    case 5u:  anim_init_frame(m, r, obj, 0, 1, 1); return 0; /* 0x1795A4 */
    case 6u:  anim_init_dir(m, r, obj, 0, 14u, 0, 1, 1); return 0; /* 0x179632 */
    case 7u:  anim_init_dir(m, r, obj, 1, 14u, 0, 1, 1); return 0; /* 0x179646 */
    case 8u:  anim_init_frame(m, r, obj, 1, 0, 0); return 0; /* 0x17955E */
    case 9u:  anim_init_dir(m, r, obj, 0,  6u, 1, 0, 0); return 0; /* 0x179600 */
    case 10u: anim_init_dir(m, r, obj, 1,  6u, 1, 0, 0); return 0; /* 0x179614 */
    case 11u: anim_init_frame(m, r, obj, 1, 1, 0); return 0; /* 0x179588 */
    case 12u: anim_init_dir(m, r, obj, 0, 14u, 1, 1, 0); return 0; /* 0x179628 */
    case 13u: anim_init_frame(m, r, obj, 1, 1, 1); return 0; /* 0x1795CA */
    case 14u: anim_init_dir(m, r, obj, 0, 16u, 1, 1, 1); return 0; /* 0x17963C */
    case 15u: anim_init_dir(m, r, obj, 1, 16u, 1, 1, 1); return 0; /* 0x179650 */
    case 16u: {                                              /* 0x17965A */
        wr16(m, obj + 0x3Eu, 1u);
        int8_t b = (int8_t)(uint8_t)(rd8(m, obj + 0x20u) - 0x20u);
        if (b < 0) b = 0;
        if (b > 0x40) b = 0x40;
        uint32_t d0 = (uint32_t)((uint8_t)(b + 2) >> 2);
        uint32_t a1 = rd32(m, obj + 0x3Au) + d0 * 6u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        uint16_t w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w);
        r->d[0] = setw(d0, w);
        r->a[1] = a1;
        return 0;
    }
    default:
        return -1;
    }
}

int Fn_196A84(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x1Cu, 0u);                            /* 0x196A84 */
    wr16(m, obj + 0x1Eu, 0u);                            /* 0x196A88 */
    wr16(m, obj + 0x22u, 0u);                            /* 0x196A8C */
    wr8(m, obj + 0x35u, 0u);                             /* 0x196A90 */
    wr32(m, obj + 0x24u, 0x00C80000u);                   /* 0x196A94 */
    Obj_18495E(m, obj);                                  /* 0x196A9C */
    wr32(m, obj + 0x28u, 0x00010000u);                   /* 0x196AA2 */
    wr32(m, obj + 0x2Cu, 0x0004000Bu);                   /* 0x196AAA */
    wr32(m, obj + 0x30u, 0x0004000Bu);                   /* 0x196AB2 */
    Obj_18495E(m, obj);                                  /* 0x196ABA */
    wr16(m, obj + 0x20u, 0u);                            /* 0x196AC0 */

    uint32_t a1 = rd32(m, obj + 0x0Cu);                  /* 0x196AC4 */
    uint16_t idx = rd16(m, a1);                          /* 0x196AC8 */
    a1 += 2u;
    r->d[0] = setw(r->d[0], idx);
    wr16(m, obj + 0x40u, idx);                           /* 0x196ACA */
    wr32(m, obj + 0x3Au, a1);                            /* 0x196ACE */
    r->a[1] = a1;

    return anim_init_regs(m, r, obj, idx);               /* 0x196AD2..0x196ADC */
}
