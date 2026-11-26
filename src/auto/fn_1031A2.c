/* ROM 0x1031A2: orbiting-formation enemy tick (A0 = object).
 * On HP <= 0 awards 0x1770 via 0x0D0E04 and spawns 0x117B44; on death or
 * A5-0x144B bit 5 spawns 0x0DC5B6 and two 0x0DC49A explosions offset by
 * (-0x100000, 0), clears +0x06, and calls Obj_StartDying (0x0CEE74).
 * Otherwise subtracts scroll Y (A5+0x4228) from +0x3E; when offscreen or
 * bomb-inactive, steps ObjScript_CF1E4 (flags7 bit 1) and ObjScript_CF4B8
 * with record 0x10336E. If +0x3A > 0x154, clears +0x06 and starts dying.
 * Otherwise syncs (+0x14, +0x18), steps script 0x0D0690, adds to collision
 * lists (0x0D0DC2), blinks colour (0x0CBE34), and while on-screen with bomb
 * active adds 0x0CF898 orbit offset to (+0x14, +0x18) and homes (+0x3A, +0x3E)
 * toward (0x71, 0x60) via 0x0D0E52 + 0x0D2D44(speed 7). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

/* ROM 0x0D0DC2 with A1 register tracking. */
static void add_coll_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_io = a1;
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

int Fn_1031A2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x1031A2 */
        d0 = 0x00001770u;                               /* 0x10327A */
        uint8_t mode = rd8(m, obj + 0x35u);
        if (mode == 1u)
            a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
        else if (mode == 2u)
            a1 = rd32(m, obj + 0x36u);
        Obj_0D0E04(m, obj, d0);
        d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0);
        goto death_explosions;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x1031AA */
        goto death_explosions;

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u)); /* 0x1031B4 */

    {
        int on_screen = ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
                         (int16_t)rd16(m, obj + 0x14u) >= -0x000A &&
                         (int16_t)rd16(m, obj + 0x14u) <= 0x011C);
        if (!on_screen) {                               /* 0x1031DA */
            uint8_t f7 = rd8(m, obj + 0x07u);
            wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
            if ((f7 & 0x02u) != 0u) {
                if (ObjScript_CF1E4(m, obj) < 0)        /* 0x1031E4 */
                    return -1;
            }
            if (rd8(m, obj + 0x20u) != 0u) {            /* 0x1031EA */
                a1 = 0x0010336Eu;
                if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)
                    return -1;
            }
        }
    }

    if ((int16_t)rd16(m, obj + 0x3Au) > 0x0154) {       /* 0x1031FC */
        wr8(m, obj + 0x06u, 0u);                        /* 0x103364 */
        Obj_StartDying(m, obj);                         /* 0x103368 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x103206 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x10320C */
    step_script_regs(m, obj, &d0, &a1);                 /* 0x103212 */
    add_coll_regs(m, obj, &a1);                         /* 0x103218 */
    Obj_BlinkColour(m, obj);                            /* 0x10321E */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&      /* 0x103224 */
        (int16_t)rd16(m, obj + 0x14u) >= -0x000A &&
        (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        a1 = rd32(m, obj + 0x0Cu);                      /* 0x103244 */
        if (Obj_0CF898(m, obj, &d0, &d1) < 0)
            return -1;
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
        int16_t dx = (int16_t)(rd16(m, obj + 0x3Au) - 0x0071);
        int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - 0x0060);
        uint16_t ang = Obj_Atan256(m, dx, dy);          /* 0x103262 */
        sub_0D2D44(m, ang, 7u, &d0, &d1);               /* 0x10326A */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_explosions:
    {
        uint32_t x = rd32(m, obj + 0x14u) + 0x002F0000u;
        uint32_t y = rd32(m, obj + 0x18u) + 0x00140000u;
        uint16_t d3w = rd16(m, obj + 0x58u);
        uint16_t d4w = rd16(m, obj + 0x60u);
        static const uint32_t fns[3] = { 0x000DC5B6u, 0x000DC49Au, 0x000DC49Au };
        for (int i = 0; i < 3; i++) {
            d0 = Obj_Spawn(m, obj, fns[i], 3u, 2u, 0u);
            if ((int32_t)d0 < 0)
                break;
            a1 = d0;
            if (i > 0)
                x -= 0x00100000u;
            wr32(m, a1 + 0x14u, x);
            wr32(m, a1 + 0x18u, y);
            wr16(m, a1 + 0x58u, d3w);
            wr16(m, a1 + 0x60u, d4w);
        }
        d1 = y;
        wr8(m, obj + 0x06u, 0u);                        /* 0x103364 */
        Obj_StartDying(m, obj);                         /* 0x103368 */
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
