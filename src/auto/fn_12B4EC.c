/* ROM 0x12B4EC: boss/ship gun/turret overlay tick (A0 = object, +0x08 =
 * parent, +0x0C = anchor segment). Frees self (0x0105EC) when parent->+0x59
 * != 0. Otherwise copies +0x14/+0x18/+0x20 from +0x0C, sets directional
 * sprite/hitbox/offsets via 0x12B41E, pushes collision/flash via 0x11B06E,
 * copies grandparent +0x1D into +0x4F, adds +0x50/+0x52 draw offset, draws
 * sprite (0x001282), fires 0x0CDDB0 projectile when A5+0x30EF bit 7 was set
 * or A5+0x30EE bit 0 + 0x0CC2BC(0x3C) == 0, and re-arms 0x12B4EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x001282 register outputs after Obj_DrawSprite(m, obj) */
static void draw_sprite_post_regs(const f3mem_t *m, uint32_t obj,
                                  uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);
    *a1_out = rd32(m, SPR_CURSOR);
    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0CC2BC: D0 = (rem << 16) | quot, D1 = divisor */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                           uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1 = (d1 << 16) | (d1 >> 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t num = rd16(m, F3_A5 + 0x0878u);
    uint16_t div = (uint16_t)d1;
    uint16_t q = (uint16_t)(num / div);
    uint16_t rem = (uint16_t)(num % div);
    *d0_out = ((uint32_t)rem << 16) | q;
    *d1_out = d1;
    return q;
}

/* ROM 0x12B41E: set sprite, flip, hitboxes and draw offset from +0x20 angle */
static void sub_12B41E(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    d0w = (uint16_t)((d0w & 0xFF00u) | (uint8_t)(d0w + 4u));
    d0w >>= 3;
    uint32_t d1 = (d0w & 0x0010u) ? 0xFFFFFFFFu : 0u;
    d0w &= 0x000Fu;

    uint32_t a1 = rd32(m, obj + 0x3Au);
    uint32_t a2 = rd32(m, a1); a1 += 4u;
    a1 += (uint32_t)(int32_t)(int16_t)d0w * 8u;
    a2 += (uint32_t)(int32_t)(int16_t)d0w * 4u;

    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));

    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, rd8(m, a1++));
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x52u, d0w);

    if ((uint16_t)d1 != 0u) {
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x03u));
        wr16(m, obj + 0x50u, (uint16_t)(-(int16_t)rd16(m, obj + 0x50u)));
        wr16(m, obj + 0x52u, (uint16_t)(-(int16_t)rd16(m, obj + 0x52u)));
    }

    *d0_io = setw(*d0_io, d0w);
    *d1_io = d1;
    *a1_out = a1;
}

int Fn_12B4EC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x12B4EC */

    uint8_t flag = rd8(m, a1 + 0x59u);                   /* 0x12B4F0 */
    wr8(m, obj + 0x59u, flag);
    if (flag != 0u) {                                    /* 0x12B4F6 */
        Obj_Free(m, obj);                                /* 0x12B5AE (preserves D0,A0,A1) */
        r->a[1] = a1;
        return 0;
    }

    a1 = rd32(m, obj + 0x0Cu);                           /* 0x12B4FA */
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));           /* 0x12B4FE */
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));           /* 0x12B504 */
    wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));           /* 0x12B50A */

    sub_12B41E(m, obj, &d0, &d1, &a1);                   /* 0x12B510 */
    wr8(m, obj + 0x35u, 0u);                             /* 0x12B514 */
    Obj_11B06E(m, obj);                                  /* 0x12B518 */

    a1 = rd32(m, obj + 0x08u);                           /* 0x12B51E */
    a1 = rd32(m, a1 + 0x08u);                            /* 0x12B522 */
    wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x1Du));             /* 0x12B526 */

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u))); /* 0x12B52C..0x12B530 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u))); /* 0x12B534..0x12B538 */
    Obj_DrawSprite(m, obj);                              /* 0x12B53C */
    draw_sprite_post_regs(m, obj, &d0, &d1, &a1);

    /* 0x12B542 bfclr (0x30ee,A5){8:1} */
    uint8_t forced = (uint8_t)(rd8(m, F3_A5 + 0x30EFu) & 0x80u);
    if (forced != 0u) {
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) & ~0x80u));
        goto spawn_shot;
    }

    /* 0x12B54C bftst (0x30ee,A5){7:1} */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x01u) != 0u) {
        d0 = setw(d0, 0x003Cu);                          /* 0x12B556 */
        if (sub_0CC2BC(m, 0x003Cu, &d0, &d1) == 0u)      /* 0x12B55A..0x12B562 */
            goto spawn_shot;
    }
    goto rearm;

spawn_shot:
    {
        a1 = 0x0012C3E2u;                                /* 0x12B566 lea (0xE7A,PC),A1 */
        uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8); /* 0x12B56C..0x12B570 */
        idx = (uint16_t)((idx & 0xFF00u) | (uint8_t)(idx + 4u)); /* 0x12B572 */
        idx >>= 3;                                       /* 0x12B574 */
        uint16_t yoff = rd16(m, a1 + (uint32_t)idx * 4u + 2u); /* 0x12B576 */
        uint16_t xoff = rd16(m, a1 + (uint32_t)idx * 4u); /* 0x12B57A */
        d1 = setw(d1, yoff);
        uint32_t d2 = ((uint32_t)(xoff & 0xFFu) << 24)
                    | ((uint32_t)(yoff & 0xFFu) << 16)
                    | (uint16_t)(rd16(m, obj + 0x20u) - 0x0800u); /* 0x12B57E..0x12B58A */
        d0 = Obj_Spawn(m, obj, 0x000CDDB0u, 5u, 2u, d2); /* 0x12B58E..0x12B5A4 */
    }

rearm:
    wr32(m, obj + 0x00u, 0x0012B4ECu);                   /* 0x12B5A8 -> 0x12B4E4 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
