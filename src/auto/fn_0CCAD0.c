/* ROM 0x0CCAD0: multi-way projectile / burst tick (A0 = object).
 * Handles bomb/stage-clear flag (A5-0x144B bit 4 -> 0x0CCB86 -> 0x0CD3D6),
 * speed decay (-0x50 on +0x22; if < 0, spawn 16x 0x11B9EE ring and switch to
 * 0x0CCB86), movement (0x0CBE12), animation (0x0CBD52 on 0x0CCB8E), debug kill
 * (A5+0x1D58 bit 1 -> 0x0CBB2C), sprite draw (0x001282), hazard list push
 * (A5+0x906), and bounds check (cmp2.w 0x0CC112 / 0x0CC116 -> 0x0CCB86). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static void draw_sprite_post_regs(const f3mem_t *m, uint32_t obj,
                                  uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);
    *a1_out = rd32(m, F3_A5 - 0x0CA0u);
    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = (*d1_io & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0CCAD0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0CCAD0: stage/bomb clear flag (A5-0x144B bit 4) -> 0x0CCB86 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto state_0CCB86;

    /* 0x0CCADA: decay +0x22 by 0x50; if < 0, burst 16 children */
    {
        int16_t v22 = (int16_t)(rd16(m, obj + 0x22u) - 0x0050);
        wr16(m, obj + 0x22u, (uint16_t)v22);
        if (v22 < 0) {
            uint32_t d1 = 0x08080000u;                        /* 0x0CCB52 */
            for (int i = 0; i < 16; i++) {
                r->d[0] = Obj_Spawn(m, obj, 0x0011B9EEu, 5u, 2u, d1);
                d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 + 0x1000u);
            }
            r->d[1] = d1;
            wr32(m, obj, 0x000CCB86u);                        /* 0x0CCB7E */
            return 0;
        }
        wr8(m, obj + 0x43u, rd8(m, obj + 0x22u));             /* 0x0CCAE4 */
    }

    /* 0x0CCAEA: move by angle (+0x20) and animate (0x0CCB8E) */
    {
        uint16_t ang = rd16(m, obj + 0x20u);
        int16_t speed = (int16_t)rd16(m, obj + 0x42u);
        int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
        r->d[1] = (uint32_t)((int32_t)speed * c);
        Obj_MoveByAngle(m, obj, ang);                         /* 0x0CCAEE -> 0x0CBE12 */
        Obj_Animate(m, obj, 0x000CCB8Eu);                     /* 0x0CCAFA -> 0x0CBD52 */
    }

    /* 0x0CCB00: debug kill check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);                          /* 0x0CCB18 -> 0x0CBB2C */
            return 0;
        }
    }

    /* 0x0CCB1E: draw sprite, push to hazard list, bounds check */
    Obj_DrawSprite(m, obj);                                   /* 0x0CCB1E -> 0x001282 */
    draw_sprite_post_regs(m, obj, &r->d[0], &r->d[1], &r->a[1]);
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x0906u);               /* 0x0CCB24 */
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
        r->a[1] = a1 + 4u;
    }

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);            /* 0x0CCB2E..0x0CCB3A */
        int16_t xlo = (int16_t)rd16(m, 0x000CC112u);
        int16_t xhi = (int16_t)rd16(m, 0x000CC114u);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)x;
        if (x < xlo || x > xhi)
            goto state_0CCB86;

        int16_t y = (int16_t)rd16(m, obj + 0x18u);            /* 0x0CCB3E..0x0CCB4A */
        int16_t ylo = (int16_t)rd16(m, 0x000CC116u);
        int16_t yhi = (int16_t)rd16(m, 0x000CC118u);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)y;
        if (y < ylo || y > yhi)
            goto state_0CCB86;
    }

    wr32(m, obj, 0x000CCAD0u);                                /* 0x0CCAC8 */
    return 0;

state_0CCB86:
    {
        uint32_t a1 = rd32(m, 0x000CD448u);                   /* 0x0CD3D6 (D0w = 1) */
        uint16_t snd = rd16(m, a1);
        a1 += 2u;
        if (snd != 0u)
            Sound_PlayHooked(m, (int16_t)snd);
        wr32(m, obj + 0x0Cu, a1);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        wr16(m, obj + 0x1Cu, 0u);
        wr32(m, obj, 0x000CD412u);
        r->a[1] = a1;
        r->d[0] = (r->d[0] & 0xFFFF0000u) | snd;
        return 0;
    }
}
