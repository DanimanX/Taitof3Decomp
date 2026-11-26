/* ROM 0x0F1922: multi-stage enemy/projectile tick (A0 = object).
 * Scrolls +0x3E, runs 0x0CF1E4/0x0CF4B8 when state < 3, culls off-screen
 * (0x0CF85A), syncs +0x14/+0x18, and dispatches state 0..5 (0x0F1984).
 * States 3 and 4 step through packed word lists in +0x1E/+0x0C with
 * 0x0D4A2C polar offsets; state 4 spawns 0x0F1B04 when +0x0C reaches -1. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CF7FE */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((f7 & ~0x80u) | 0x02u));
}

/* ROM 0x0D4A2C: (table[d2w] * d0w swapped, table[d2w] * d1w swapped). */
static void sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)(int16_t)d2w * 4u);
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

int Fn_0F1922(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F1922 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F1926 */

    if (rd8(m, obj + 0x06u) < 3u) {                     /* 0x0F192A */
        uint8_t f7 = rd8(m, obj + 0x07u);               /* 0x0F1934 */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F193E */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x0F1944 */
            a1 = 0x000EABE2u;                           /* 0x0F194C */
            if (ObjScript_CF4B8(m, obj, a1) < 0)        /* 0x0F1950 */
                return -1;
        }
    }

    Obj_FlagOffscreen(m, obj);                          /* 0x0F1956 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0F195C */
        Obj_StartDying(m, obj);                         /* 0x0F1962 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F1968 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F196E */

    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F1974..0x0F1976 */
    switch ((uint16_t)d0) {                             /* 0x0F197A -> 0x0F1984 */
    case 0u:                                            /* 0x0F199C */
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            wr8(m, obj + 0x06u, 2u);
            if (rd8(m, obj + 0x4Eu) == 0u)
                wr8(m, obj + 0x06u, 1u);
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;
    case 1u:                                            /* 0x0F19C0 */
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;
    case 2u: {                                          /* 0x0F19C8 */
        uint8_t c = (uint8_t)(rd8(m, obj + 0x4Eu) - 1u);
        wr8(m, obj + 0x4Eu, c);
        if (c == 0u) {
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - 7u));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 7u));
            wr8(m, obj + 0x06u, 3u);
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;
    }
    case 3u: {                                          /* 0x0F19E6 */
        uint16_t v = (uint16_t)(rd16(m, obj + 0x1Eu) - 0x0404u);
        wr16(m, obj + 0x1Eu, v);
        if (v == 0u) {
            wr16(m, obj + 0x0Cu, 0u);
            wr8(m, obj + 0x06u, 4u);
        } else {
            uint16_t ang = (uint8_t)(~rd8(m, obj + 0x1Eu));
            sub_0D4A2C(m, 0x0010u, 0x0010u, ang, &d0, &d1);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
        }
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;
    }
    case 4u: {                                          /* 0x0F1A24 */
        wr8(m, obj + 0x1Du, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        uint16_t d2w = (uint16_t)(rd16(m, obj + 0x0Cu) + 0x0505u);
        if (d2w == 0xFFFFu) {
            d0 = Obj_Spawn(m, obj, 0x000F1B04u, 5u, 2u, 0u);
            if ((int32_t)d0 < 0) {
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
            } else {
                a1 = d0;
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                wr8(m, a1 + 0x4Eu, 0u);
                wr8(m, obj + 0x23u, 1u);
                wr8(m, obj + 0x06u, 5u);
            }
        } else {
            wr16(m, obj + 0x0Cu, d2w);
            wr16(m, obj + 0x1Eu, d2w);
            uint16_t ang = (uint8_t)(~(d2w & 0xFFu));
            sub_0D4A2C(m, 0x0010u, 0x0010u, ang, &d0, &d1);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
            wr8(m, obj + 0x1Du, 0xDEu);
            draw_sprite_regs(m, obj, &d0, &d1, &a1);
        }
        break;
    }
    case 5u: {                                          /* 0x0F1ABA */
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c == 0u)
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
        break;
    }
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
