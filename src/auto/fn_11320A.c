/* ROM 0x11320A: homing/orbiting sub-object tick (A0 = object, A0+0x08 = parent).
 * Frees itself with sounds 0x2E/0x0D if parent+0x50 != 0. Otherwise snaps to
 * parent's position, steps animation 0x11333A, and loops up to 20 steps moving
 * toward (grandparent pos + parent+0x52/+0x54) with RNG jitter for the first 10
 * steps, drawing each step until within +-16 px or 20 steps, then parks on
 * 0x113300. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
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

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle]; D1 = speed * cos[angle]. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint16_t angle)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x027E40u + 0x80u + 2u * angle);
    int16_t s = (int16_t)rd16(m, 0x027E40u + 2u * angle);
    uint32_t dx = (uint32_t)((int32_t)speed * (int32_t)c);
    uint32_t dy = (uint32_t)((int32_t)speed * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
    r->d[1] = dx;
}

int Fn_11320A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x11320A */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x50u) != 0u) {                     /* 0x11320E..0x113212 */
        int16_t c = Sound_PreCmdHook(m, 0x002E);        /* 0x113308 */
        Sound_PlayCmdForced(m, c);                      /* 0x11330E */
        Sound_PlayHooked(m, 0x000Du);                   /* 0x11331C + 0x113322 */
        Obj_Free(m, obj);                               /* 0x11332E */
        return 0;                                       /* 0x113338 */
    }

    /* 0x113216..0x113236 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x06u, 0u);
    r->a[1] = 0x0011333Au;                              /* 0x11322A */
    Obj_Animate(m, obj, 0x0011333Au);                   /* 0x113230 */
    r->d[0] = setw(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));
    wr32(m, obj + 0x36u, rd32(m, obj + 0x10u));         /* 0x113236 */

    for (;;) {
        a1 = rd32(m, obj + 0x08u);                      /* 0x11323C */
        r->a[1] = a1;
        uint32_t a2 = rd32(m, a1 + 0x08u);              /* 0x113240 */
        int16_t dx = (int16_t)(rd16(m, a2 + 0x14u) + rd16(m, a1 + 0x52u) - rd16(m, obj + 0x14u));
        int16_t dy = (int16_t)(rd16(m, a2 + 0x18u) + rd16(m, a1 + 0x54u) - rd16(m, obj + 0x18u));
        r->d[0] = setw(r->d[0], (uint16_t)dx);
        r->d[1] = setw(r->d[1], (uint16_t)dy);

        /* 0x11325C..0x11327C */
        if (dx <= 0x10 && dx >= -0x10 && dy <= 0x10 && dy >= -0x10)
            break;

        /* 0x113280 */
        uint16_t ang = Polar_Angle256(m, dx, dy);
        r->d[0] = setw(r->d[0], ang);
        r->d[1] = setw(r->d[1], ang);                   /* 0x113286 */

        uint16_t step = (uint16_t)(rd16(m, obj + 0x06u) + 1u); /* 0x113288 */
        wr16(m, obj + 0x06u, step);
        if ((int16_t)step > 0x14)                       /* 0x11328C..0x113292 */
            break;

        if ((int16_t)step <= 0x0A) {                    /* 0x113296..0x11329C */
            uint32_t rng = Rng_NextMem(m);              /* 0x1132A0 */
            uint8_t j = (uint8_t)((rng & 0x3Fu) - 0x20u); /* 0x1132A6..0x1132AA */
            r->d[0] = (rng & 0xFFFFFF00u) | j;
            r->d[1] = (r->d[1] & 0xFFFFFF00u) | (uint8_t)((uint8_t)r->d[1] + j); /* 0x1132AE */
        }

        /* 0x1132B0..0x1132B6 */
        uint16_t d1w = (uint16_t)r->d[1];
        wr16(m, obj + 0x20u, d1w);
        r->d[0] = setw(r->d[0], d1w);
        move_by_angle_regs(m, r, d1w);

        /* 0x1132BC..0x1132DE */
        uint16_t d0w = rd16(m, obj + 0x20u);
        d0w = (uint16_t)((d0w & 0xFF00u) | (uint8_t)(d0w + 0x48u));
        uint32_t d0 = setw(r->d[0], d0w);
        uint32_t d1 = (d0 >> 6) & 0x03u;                /* 0x1132C4 bfextu D0{24:2},D1 */
        d0w = (d0w >> 4) & 0x07u;                       /* 0x1132C8..0x1132CA */
        uint32_t base = rd32(m, obj + 0x36u);           /* 0x1132CE */
        wr32(m, obj + 0x10u, rd32(m, base + (uint32_t)d0w * 4u)); /* 0x1132D2 */
        wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + d1));  /* 0x1132DE */
        r->d[0] = setw(d0, d0w);
        r->d[1] = d1;

        draw_sprite_regs(m, r);                         /* 0x1132E4 */

        d0w = rd16(m, obj + 0x20u);                     /* 0x1132EA */
        r->d[0] = setw(r->d[0], d0w);
        move_by_angle_regs(m, r, d0w);                  /* 0x1132EE */
    }

    wr32(m, obj, 0x00113300u);                          /* 0x1132F8 */
    return 0;
}
