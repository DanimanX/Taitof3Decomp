/* ROM 0x0F5532: expanding/deploying enemy tick (A0 = object).
 * Subtracts vertical scroll (A5+0x4228), runs 0x0CF1E4 and 0x0CF4B8 (0x0F8214)
 * when state < 3, checks offscreen (0x0CF85A -> 0x0CEE74), and dispatches
 * states 0..5 (jump table 0x0F5594): arm on screen (0x0CF7FE), countdown,
 * zoom-in/out using 0x0D4A2C, spawning 0x0F5718 on state 4 completion. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

#define REC_0F8214 0x000F8214u

/* ROM 0x0D4A2C: (D0w, D1w) = high words of tbl[D2w] * d0w, tbl[D2w] * d1w. */
static void sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t d4 = rd32(m, 0x000D4A5Au + (uint32_t)(int32_t)(int16_t)d2w * 4u);
    uint32_t p1 = d4 * (uint32_t)d1w;
    uint32_t p0 = d4 * (uint32_t)d0w;
    *out_d1 = (p1 << 16) | (p1 >> 16);
    *out_d0 = (p0 << 16) | (p0 >> 16);
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0F5532(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        uint32_t dy = rd32(m, F3_A5 + 0x4228u);               /* 0x0F5532 */
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - dy);
        r->d[0] = dy;
    }

    if (rd8(m, obj + 0x06u) < 3u) {                           /* 0x0F553A */
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {
            r->a[1] = REC_0F8214;
            if (ObjScript_CF4B8(m, obj, REC_0F8214) < 0)
                return -1;
        }
    }

    Obj_FlagOffscreen(m, obj);                                /* 0x0F5566 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    {
        uint8_t st = rd8(m, obj + 0x06u);
        r->d[0] = (r->d[0] & 0xFFFF0000u) | st;
        switch (st) {
        case 0:                                               /* 0x0F55AC */
            Obj_ArmOnScreen(m, obj);
            if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
                wr8(m, obj + 0x06u, 2u);
                if (rd8(m, obj + 0x4Eu) == 0u)
                    wr8(m, obj + 0x06u, 1u);
            }
            draw_sprite_regs(m, obj, r);
            break;
        case 1:                                               /* 0x0F55D0 */
            draw_sprite_regs(m, obj, r);
            break;
        case 2:                                               /* 0x0F55D8 */
            {
                uint8_t t = (uint8_t)(rd8(m, obj + 0x4Eu) - 1u);
                wr8(m, obj + 0x4Eu, t);
                if (t == 0u) {
                    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - 0x000Eu));
                    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 0x0009u));
                    wr8(m, obj + 0x06u, 3u);
                }
            }
            draw_sprite_regs(m, obj, r);
            break;
        case 3:                                               /* 0x0F55FA */
            {
                uint16_t z = (uint16_t)(rd16(m, obj + 0x1Eu) - 0x0404u);
                wr16(m, obj + 0x1Eu, z);
                if (z == 0u) {
                    wr16(m, obj + 0x0Cu, 0u);
                    wr8(m, obj + 0x06u, 4u);
                } else {
                    uint8_t d2b = (uint8_t)(~rd8(m, obj + 0x1Eu));
                    uint32_t ox = 0, oy = 0;
                    sub_0D4A2C(m, 0x1Du, 0x14u, d2b, &ox, &oy);
                    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)ox));
                    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)oy));
                }
            }
            draw_sprite_regs(m, obj, r);
            break;
        case 4:                                               /* 0x0F5638 */
            wr8(m, obj + 0x1Du, 0u);
            wr16(m, obj + 0x1Eu, 0u);
            draw_sprite_regs(m, obj, r);
            {
                uint16_t d2w = (uint16_t)(rd16(m, obj + 0x0Cu) + 0x0505u);
                if (d2w == 0xFFFFu) {
                    uint32_t child = Obj_Spawn(m, obj, 0x000F5718u, 5u, 2u, 0u);
                    r->d[0] = child;
                    if ((int32_t)child >= 0) {
                        r->a[1] = child;
                        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                        wr8(m, child + 0x4Eu, 0u);
                        wr8(m, obj + 0x23u, 1u);
                        wr8(m, obj + 0x06u, 5u);
                    } else {
                        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
                    }
                } else {
                    wr16(m, obj + 0x0Cu, d2w);
                    wr16(m, obj + 0x1Eu, d2w);
                    uint8_t d2b = (uint8_t)(~(d2w & 0xFFu));
                    uint32_t ox = 0, oy = 0;
                    sub_0D4A2C(m, 0x1Du, 0x14u, d2b, &ox, &oy);
                    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)ox));
                    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)oy));
                    wr8(m, obj + 0x1Du, 0xDEu);
                    draw_sprite_regs(m, obj, r);
                }
            }
            break;
        case 5:                                               /* 0x0F56CE */
            {
                uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
                wr8(m, obj + 0x23u, t);
                if (t == 0u)
                    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
            }
            draw_sprite_regs(m, obj, r);
            break;
        default:
            break;
        }
    }
    return 0;
}
