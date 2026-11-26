/* ROM 0x105356: orbiting/ring enemy part tick (A0 = object).
 * If +0x24 <= 0, relays timer 0x1770 (0x0D0E04), spawns score 0x117B44 and
 * explosion 0x0DC5B6, clears +0x06 and dies (0x0CEE74). On bomb-clear
 * (A5-0x144B bit 5), spawns 0x0DC5B6, clears +0x06 and dies. Scrolls +0x3E,
 * runs ObjScript_CF1E4 / ObjScript_CF4B8 (0x105604) unless bomb-pause is
 * active in [-0x0A, 0x11C], culls off-screen (0x0CF85A -> 0x0CEE74), syncs
 * +0x14/+0x18, dispatches state 1..5 (0x1054BA), steps script (0x0D0690),
 * adds to collision lists (0x0D0DC2), blinks (0x0CBE34), and unless
 * bomb-paused in [-0x0A, 0x11C], orbits via 0x0CF898 and drifts toward
 * (0x90, 0x64) via Obj_Atan256 (0x0D0E52) + 0x0D2D44 (speed 7). */
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

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w (via 0x0D4F26). */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    *out_d0 = rd32(m, 0x000D2D64u + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2ECCu + off) * (uint32_t)d1w;
    return 0;
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *a1_io)
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

/* ROM 0x0CF4B8 wrapper tracking D0/D1 left by sub_0D2D44 in modes 12..15. */
static int cf4b8_regs(const f3mem_t *m, uint32_t obj, uint32_t rec,
                      uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t op = rd8(m, obj + 0x20u);
    if (ObjScript_CF4B8(m, obj, rec) < 0)
        return -1;
    if (op >= 12u && op <= 15u)
        return sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), d0_io, d1_io);
    return 0;
}

static int in_bomb_pause(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        return 0;
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    return (x >= -0x0A && x <= 0x011C);
}

int Fn_105356(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x105356..0x10535A */
        d0 = 0x00001770u;                               /* 0x105442 */
        Obj_0D0E04(m, obj, d0);                         /* 0x105448 */
        d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0); /* 0x10545E */
        goto spawn_explode;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x10535E */
        goto spawn_explode;

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x105368 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x10536C */

    if (!in_bomb_pause(m, obj)) {                       /* 0x105370..0x105388 */
        uint8_t f7 = rd8(m, obj + 0x07u);               /* 0x10538E */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {                       /* 0x105394 */
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x105398 */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x10539E */
            a1 = 0x00105604u;                           /* 0x1053A6 */
            if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)   /* 0x1053AA */
                return -1;
        }
    }

    Obj_FlagOffscreen(m, obj);                          /* 0x1053B0 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x1053B6..0x1053BC */
        wr8(m, obj + 0x06u, 0u);                        /* 0x1054B0 */
        Obj_StartDying(m, obj);                         /* 0x1054B4 */
        goto done;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x1053C0 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x1053C6 */
    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x1053CC..0x1053CE */

    switch ((uint16_t)d0) {                             /* 0x1053D2 -> 0x1054BA */
    case 1u:                                            /* 0x1054D2 */
        break;
    case 2u: {                                          /* 0x1054D4 */
        uint16_t v = (uint16_t)(rd16(m, obj + 0x42u) + 1u);
        wr16(m, obj + 0x42u, v);
        if (v == 8u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
            wr8(m, obj + 0x06u, 4u);
        }
        break;
    }
    case 3u: {                                          /* 0x1054F0 */
        wr8(m, obj + 0x4Fu, 0u);
        uint16_t v = (uint16_t)(rd16(m, obj + 0x42u) - 1u);
        wr16(m, obj + 0x42u, v);
        if (v == 0u) {
            wr8(m, obj + 0x23u, 0u);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
            wr8(m, obj + 0x06u, 5u);
        }
        break;
    }
    case 4u:                                            /* 0x10550E */
        wr8(m, obj + 0x4Fu, 0u);
        if ((int16_t)rd16(m, obj + 0x68u) >= 0) {
            uint32_t sp = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            d0 = sp;
            if ((int32_t)sp >= 0) {
                a1 = sp;
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
                wr16(m, a1 + 0x58u, rd16(m, obj + 0x68u));
                wr16(m, a1 + 0x10u, 0x000Eu);
                wr8(m, obj + 0x4Fu, 0x70u);
            }
        }
        break;
    case 5u: {                                          /* 0x10556A */
        uint8_t cd = rd8(m, obj + 0x23u);
        if (cd != 0u) {
            cd = (uint8_t)(cd - 1u);
            wr8(m, obj + 0x23u, cd);
            if (cd != 0u)
                break;
        }
        if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)
            break;
        uint16_t rnd = Obj_Next4Way(m);
        uint16_t diff = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
        d0 = setw(d0, (uint16_t)(rnd + diff));
        d1 = setw(d1, diff);
        if ((uint16_t)d0 < 0x00FAu)
            break;
        uint32_t sp = Obj_Spawn(m, obj, 0x000DB426u, 9u, 2u, 0u);
        d0 = sp;
        if ((int32_t)sp < 0)
            break;
        a1 = sp;
        wr16(m, a1 + 0x10u, 0x0010u);
        wr16(m, a1 + 0x36u, 0xFFFAu);
        wr8(m, obj + 0x23u, 0x14u);
        sp = Obj_Spawn(m, obj, 0x000DB80Eu, 9u, 2u, 0u);
        d0 = sp;
        if ((int32_t)sp < 0)
            break;
        a1 = sp;
        wr16(m, a1 + 0x10u, 0x0010u);
        wr16(m, a1 + 0x36u, 0x0025u);
        break;
    }
    default:
        return -1;
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x1053DA */
    Obj_0D0DC2(m, obj);                                 /* 0x1053E0 */
    a1 = rd32(m, F3_A5 + 0x0906u);
    Obj_BlinkColour(m, obj);                            /* 0x1053E6 */

    if (!in_bomb_pause(m, obj))                         /* 0x1053EC..0x105404 */
        goto done;                                      /* 0x10540A */

    if (Obj_0CF898(m, obj, &d0, &d1) < 0)               /* 0x10540C */
        return -1;
    a1 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);    /* 0x105412 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);    /* 0x105416 */

    int16_t dx = (int16_t)(rd16(m, obj + 0x3Au) - 0x0090u); /* 0x10541A..0x10541E */
    int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - 0x0064u); /* 0x105422..0x105426 */
    d0 = setw(d0, Obj_Atan256(m, dx, dy));              /* 0x10542A */
    d1 = 7u;                                            /* 0x105430 */
    if (sub_0D2D44(m, (uint16_t)d0, (uint16_t)d1, &d0, &d1) < 0) /* 0x105432 */
        return -1;
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);    /* 0x105438 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);    /* 0x10543C */
    goto done;

spawn_explode:                                          /* 0x105468 */
    {
        uint32_t sp = Obj_Spawn(m, obj, 0x000DC5B6u, 3u, 2u, 0u);
        d0 = sp;
        if ((int32_t)sp >= 0) {
            a1 = sp;
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
    }
    wr8(m, obj + 0x06u, 0u);                            /* 0x1054B0 */
    Obj_StartDying(m, obj);                             /* 0x1054B4 */

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
