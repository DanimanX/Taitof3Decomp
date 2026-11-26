/* ROM 0x110C9C: detachable turret/pod object tick (A0 = object, A0+0x08 = parent).
 * While attached (+0x06 != 0), tracks parent+0x14/+0x18 + (0x97, 0x69), dies if
 * X < -0x40 or parent+0x06 == 0, detaches when parent+0x06 == 2 (playing sound
 * 0x1A, picking random launch velocity via 0x0D0E3A + 0x0D2D44, spawning a
 * 0x0DC36C flash), otherwise mirrors parent+0x20 tilt colour and draws. Once
 * detached (+0x06 == 0), falls with gravity +0x36B0/frame until Y > 0xE8. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0D2D44: scale (0x0D2D64[angle], 0x0D2ECC[angle]) by speed (0..50). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

int Fn_110C9C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd8(m, obj + 0x06u) != 0u) {                    /* 0x110C9C..0x110CA0 */
        uint32_t a1 = rd32(m, obj + 0x08u);             /* 0x110CA4 */
        r->a[1] = a1;
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));      /* 0x110CA8 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0097u)); /* 0x110CAE */
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));      /* 0x110CB4 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0069u)); /* 0x110CBA */

        if ((int16_t)rd16(m, obj + 0x14u) < -0x40) {    /* 0x110CC0..0x110CC6 */
            Obj_StartDying(m, obj);
            return 0;
        }

        uint8_t d0b = rd8(m, a1 + 0x06u);               /* 0x110CCC */
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | d0b;
        if (d0b == 0u) {                                /* 0x110CD0 */
            Obj_StartDying(m, obj);
            return 0;
        }

        if (d0b != 2u) {                                /* 0x110CD6..0x110CDA */
            wr8(m, obj + 0x1Du, 0u);                    /* 0x110D9A */
            int8_t v20 = (int8_t)rd8(m, a1 + 0x20u);    /* 0x110D9E */
            if (v20 > 0)
                wr8(m, obj + 0x1Du, 0x40u);             /* 0x110DAA */
            else if (v20 < 0)
                wr8(m, obj + 0x1Du, 0x42u);             /* 0x110DB4 */
            draw_sprite_regs(m, r);                     /* 0x110DBA */
            return 0;                                   /* 0x110DC0 */
        }

        /* 0x110CDE: detach */
        wr8(m, obj + 0x1Du, 0u);
        wr8(m, obj + 0x06u, 0u);
        Sound_PlayHooked(m, 0x001Au);                   /* 0x110CE6..0x110CF6 */

        uint16_t d1w = (uint16_t)((Rng_Mix1D60(m) & 7u) + 0x14u); /* 0x110CFA..0x110D08 */
        uint16_t d0w = (uint16_t)((Rng_Mix1D60(m) & 0x1Fu) + 0x3Bu); /* 0x110D0A..0x110D14 */
        uint32_t vx, vy;
        sub_0D2D44(m, d0w, d1w, &vx, &vy);              /* 0x110D18 */
        r->d[0] = vx;
        r->d[1] = vy;
        wr32(m, obj + 0x50u, vx);                       /* 0x110D1E */
        wr32(m, obj + 0x54u, vy);                       /* 0x110D22 */

        uint32_t child = Obj_Spawn(m, obj, 0x000DC36Cu, 3u, 2u, 0u); /* 0x110D26..0x110D3E */
        r->d[0] = child;
        if ((int32_t)child >= 0) {                      /* 0x110D42..0x110D44 */
            r->a[1] = child;                            /* 0x110D48 */
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0020u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u));
            wr16(m, child + 0x58u, 0u);
            wr16(m, child + 0x60u, 0u);
        }
    }

    /* 0x110D6A: ballistic fall */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x50u));
    uint32_t dy = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    r->d[0] = dy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);

    if ((int16_t)rd16(m, obj + 0x18u) > 0x00E8) {       /* 0x110D7E..0x110D84 */
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u); /* 0x110D8A */
    draw_sprite_regs(m, r);                             /* 0x110D92 */
    return 0;                                           /* 0x110D98 */
}
