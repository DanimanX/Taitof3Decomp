/* ROM 0x10F38A: falling debris/fragment spawner init (A0 = object).
 * Jitters X by (rng&0x0F)-0x17 and Y by -0x0F (0x0D0E3A), seeds stats from
 * 0x10F536 (0x0CEDE4), picks one of 4 behaviour records at 0x10F526 (0x0D0678),
 * picks random upward/downward angle (0x3A..0x79) and speed (0x19..0x20) via
 * 0x0D2D44 into (+0x50, +0x54), configures flags +0x34, subtracts stage scroll
 * (A5+0x4224/4228), draws sprite (0x001282), and installs tick 0x10F41A. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
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
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
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

int Fn_10F38A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        uint16_t dx = (uint16_t)((Obj_Next4Way(m) & 0x000Fu) - 0x0017u); /* 0x10F38A..0x10F394 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx));     /* 0x10F398 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x000Fu)); /* 0x10F39C */
    }

    Obj_0CEDE4(m, obj, 0x0010F536u);                     /* 0x10F3A2..0x10F3A6 */

    {
        uint16_t idx = (uint16_t)(Obj_Next4Way(m) & 3u); /* 0x10F3AC..0x10F3B2 */
        Obj_StartBehaviour(m, obj, rd32(m, 0x0010F526u + (uint32_t)idx * 4u)); /* 0x10F3B6..0x10F3BE */
    }

    {
        uint16_t spd = (uint16_t)((Obj_Next4Way(m) & 7u) + 0x0019u);     /* 0x10F3C4..0x10F3D2 */
        uint16_t ang = (uint16_t)((Obj_Next4Way(m) & 0x003Fu) + 0x003Au); /* 0x10F3D4..0x10F3DE */
        uint32_t vx, vy;
        sub_0D2D44(m, ang, spd, &vx, &vy);               /* 0x10F3E2 */
        wr32(m, obj + 0x50u, vx);                        /* 0x10F3E8 */
        wr32(m, obj + 0x54u, vy);                        /* 0x10F3EC */
    }

    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u)); /* 0x10F3F0..0x10F3F6 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u)); /* 0x10F3FC..0x10F400 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u)); /* 0x10F404..0x10F408 */

    draw_sprite_regs(m, r);                              /* 0x10F40C */
    wr32(m, obj, 0x0010F41Au);                           /* 0x10F412 */
    return 0;
}
