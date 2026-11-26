/* ROM 0x0CC5CA: homing/angled projectile tick (A0 = object).
 * Handles bomb/stage-clear flag (A5-0x144B bit 4 -> 0x0CD3D6 sound + switch to
 * 0x0CD412), HP <= 0 (0x0CBDDE score + explosion setup -> 0x0CC69E), speed
 * decay (-0x50 on +0x22), movement (0x0CBE12), animation (0x0CBD52 on
 * 0x0CC710), debug kill (A5+0x1D58 bit 1 -> 0x0CBD12 -> 0x0CBB2C), flash/draw
 * (0x11C32C, 0x001282), collision list push (0x0CBDA4), and countdown (+0x06). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
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

int Fn_0CC5CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0CC5CA: stage/bomb clear flag (A5-0x144B bit 4) -> 0x0CC6F8 -> 0x0CD3D6 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
        uint32_t a1 = rd32(m, 0x000CD444u);                   /* 0x0CD3D6 (D0w = 0) */
        uint16_t snd = rd16(m, a1);
        a1 += 2u;
        if (snd != 0u)
            Sound_PlayHooked(m, (int16_t)snd);                /* 0x0CD3E4..0x0CD3F2 */
        wr32(m, obj + 0x0Cu, a1);                             /* 0x0CD3F6 */
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        wr16(m, obj + 0x1Cu, 0u);
        wr32(m, obj, 0x000CD412u);                            /* 0x0CD40A */
        r->a[1] = a1;
        r->d[0] = (r->d[0] & 0xFFFF0000u) | snd;
        return 0;
    }

    /* 0x0CC5D4: HP <= 0 -> 0x0CC64C */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);                     /* 0x0CC64C -> 0x0CBDDE */
        uint32_t d0 = rd16(m, obj + 0x4Cu);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
        r->a[1] = a1;
        r->d[0] = d0;
        goto explode_0CC65C;
    }

    /* 0x0CC5DC: decay +0x22 by 0x50, clamp at 0, copy high byte to +0x43 */
    {
        int16_t v22 = (int16_t)(rd16(m, obj + 0x22u) - 0x0050);
        if (v22 < 0)
            v22 = 0;
        wr16(m, obj + 0x22u, (uint16_t)v22);
        wr8(m, obj + 0x43u, rd8(m, obj + 0x22u));             /* 0x0CC5EE */
    }

    /* 0x0CC5F4: move by angle (+0x20) and animate (0x0CC710) */
    {
        uint16_t ang = rd16(m, obj + 0x20u);
        int16_t speed = (int16_t)rd16(m, obj + 0x42u);
        int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
        r->d[1] = (uint32_t)((int32_t)speed * c);
        Obj_MoveByAngle(m, obj, ang);                         /* 0x0CC5F8 -> 0x0CBE12 */
        Obj_Animate(m, obj, 0x000CC710u);                     /* 0x0CC604 -> 0x0CBD52 */
    }

    /* 0x0CC60A: debug kill check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x0CC622 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);           /* 0x0CBD12 */
            uint32_t d0 = rd16(m, obj + 0x4Cu);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
            r->a[1] = a1;
            r->d[0] = d0;
            Obj_ExplodeTick(m, obj);                          /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    /* 0x0CC62E: flash, draw sprite, push to lists, decrement +0x06 */
    Obj_11C32C(m, obj);                                       /* 0x0CC62E -> 0x11C32C */
    Obj_DrawSprite(m, obj);                                   /* 0x0CC634 -> 0x001282 */
    draw_sprite_post_regs(m, obj, &r->d[0], &r->d[1], &r->a[1]);
    {
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);               /* 0x0CBDA4 */
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);                        /* 0x0CBDAE */
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
        r->a[1] = a1 + 4u;
    }

    {
        int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1);    /* 0x0CC640 */
        wr16(m, obj + 0x06u, (uint16_t)cnt);
        if (cnt > 0) {
            wr32(m, obj, 0x000CC5CAu);                        /* 0x0CC5C2 */
            return 0;
        }
    }

explode_0CC65C:
    wr16(m, obj + 0x2Cu, 0x0004u);
    wr16(m, obj + 0x2Eu, 0x001Bu);
    wr16(m, obj + 0x30u, 0x0004u);
    wr16(m, obj + 0x32u, 0x001Bu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x1Du, 0x1Cu);
    wr8(m, obj + 0x34u, rd8(m, obj + 0x34u) | 0x02u);
    wr16(m, obj + 0x06u, 0x0028u);
    wr32(m, obj, 0x000CC69Eu);                                /* 0x0CC696 */
    return 0;
}
