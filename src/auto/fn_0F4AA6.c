/* ROM 0x0F4AA6: enemy splitter/hatch tick (A0 = object).
 * Scrolls Y (A5+0x4228), runs script/motion (0x0CF1E4, 0x0CF4B8 with
 * 0x0F8214) when +0x06 < 3, checks off-screen (0x0CF85A -> 0x0CEE74), and
 * dispatches 6 states via 0x0F4B08: 0=activate (0x0CF7FE), 1=idle draw,
 * 2=countdown +0x4E then offset (-14, -9), 3=scale-out +0x1E via 0x0D4A2C,
 * 4=scale-in +0x0C via 0x0D4A2C then spawn 0x0F4C8C (0x01057C), 5=wait +0x23
 * then flag off-screen. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CF7FE: activate object once +0x3A enters [-0x40, 0x14F]. */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if ((flags & 0x80u) == 0u)
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;

    wr8(m, obj + 0x07u, (uint8_t)((flags & (uint8_t)~0x80u) | 0x02u));
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

/* ROM 0x0CF4B8 wrapper tracking D0/D1 left by sub_0D2D44 in modes 12..15. */
static int cf4b8_regs(const f3mem_t *m, uint32_t obj, uint32_t rec,
                      uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t op = rd8(m, obj + 0x20u);
    if (ObjScript_CF4B8(m, obj, rec) < 0)
        return -1;
    if (op >= 12u && op <= 15u)
        sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), d0_io, d1_io);
    return 0;
}

/* ROM 0x0D4A2C: (table[d2w] * d0w swapped, table[d2w] * d1w swapped). */
static void sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)(int32_t)(int16_t)d2w * 4u);
    uint32_t p1 = v * (uint32_t)d1w;
    uint32_t p0 = v * (uint32_t)d0w;
    *out_d1 = (p1 << 16) | (p1 >> 16);
    *out_d0 = (p0 << 16) | (p0 >> 16);
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

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

int Fn_0F4AA6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F4AA6 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F4AAA */

    if (rd8(m, obj + 0x06u) < 3u) {                     /* 0x0F4AAE */
        uint8_t f7 = rd8(m, obj + 0x07u);               /* 0x0F4AB8 */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if ((f7 & 0x02u) != 0u) {                       /* 0x0F4ABE */
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F4AC2 */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x0F4AC8 */
            a1 = 0x000F8214u;                           /* 0x0F4AD0 */
            if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)   /* 0x0F4AD4 */
                return -1;
        }
    }

    Obj_FlagOffscreen(m, obj);                          /* 0x0F4ADA */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0F4AE0 */
        Obj_StartDying(m, obj);                         /* 0x0F4AE6 */
        goto done;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F4AEC */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F4AF2 */
    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F4AF8..0x0F4AFA */

    switch ((uint16_t)d0) {                             /* 0x0F4AFE -> 0x0F4B08 */
    case 0u:                                            /* 0x0F4B20 */
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            wr8(m, obj + 0x06u, 2u);
            if (rd8(m, obj + 0x4Eu) == 0u)
                wr8(m, obj + 0x06u, 1u);
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;

    case 1u:                                            /* 0x0F4B44 */
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;

    case 2u:                                            /* 0x0F4B4C */
        {
            uint8_t c = (uint8_t)(rd8(m, obj + 0x4Eu) - 1u);
            wr8(m, obj + 0x4Eu, c);
            if (c == 0u) {
                wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - 0x0Eu));
                wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 0x09u));
                wr8(m, obj + 0x06u, 3u);
            }
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;

    case 3u:                                            /* 0x0F4B6E */
        {
            uint16_t w = (uint16_t)(rd16(m, obj + 0x1Eu) - 0x0404u);
            wr16(m, obj + 0x1Eu, w);
            if (w == 0u) {
                wr16(m, obj + 0x0Cu, 0u);
                wr8(m, obj + 0x06u, 4u);
            } else {
                uint16_t idx = (uint8_t)(~(uint8_t)w);
                sub_0D4A2C(m, 0x001Du, 0x0014u, idx, &d0, &d1);
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
                wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
            }
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;

    case 4u:                                            /* 0x0F4BAC */
        wr8(m, obj + 0x1Du, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        {
            uint16_t d2w = (uint16_t)(rd16(m, obj + 0x0Cu) + 0x0505u);
            if (d2w == 0xFFFFu) {
                uint32_t sp = Obj_Spawn(m, obj, 0x000F4C8Cu, 5u, 2u, 0u);
                d0 = sp;
                if ((int32_t)sp < 0) {
                    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
                } else {
                    a1 = sp;
                    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                    wr8(m, a1 + 0x4Eu, 0u);
                    wr8(m, obj + 0x23u, 2u);
                    wr8(m, obj + 0x06u, 5u);
                }
            } else {
                wr16(m, obj + 0x0Cu, d2w);
                wr16(m, obj + 0x1Eu, d2w);
                uint16_t idx = (uint8_t)(~(uint8_t)d2w);
                sub_0D4A2C(m, 0x001Du, 0x0014u, idx, &d0, &d1);
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
                wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
                wr8(m, obj + 0x1Du, 0xDEu);
                draw_sprite_regs(m, obj, &d0, &d1, &a1);
            }
        }
        break;

    case 5u:                                            /* 0x0F4C42 */
        {
            uint8_t cd = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, cd);
            if (cd == 0u)
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;

    default:
        return -1;
    }

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
