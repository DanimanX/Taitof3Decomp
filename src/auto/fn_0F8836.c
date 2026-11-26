/* ROM 0x0F8836: walking ground-turret enemy tick (A0 = object).
 * On bomb-hit (A5+0x1D58 bit 1 with X in [-10, 284]) awards 0x320 timer via
 * 0x0D0E30, clears +0x06, and tail-calls 0x0CEF0E. On HP <= 0 awards 0x320
 * via 0x0D0E04; on death or A5-0x144B bit 5 spawns 0x0DC456, clears +0x06,
 * and calls Obj_StartDying (0x0CEE74). Otherwise decrements +0x0C timer,
 * scrolls +0x3E by A5+0x4228, dispatches +0x06 via 0x0F892E (1: script/walk-in,
 * 2: patrol/terrain check 0x0C6632 + aim 0x0D07B4, 3: fire 0x0D9992, 4: scroll
 * off), syncs (+0x14, +0x18), checks offscreen (0x0CF85A), steps script
 * (0x0D0690), adds to collision lists (0x0D0DC2), and blinks colour (0x0CBE34). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../player/player_move.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02 */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t idx = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)idx * 2u);
}

/* ROM 0x0D07E4 + 0x0D08A0: test one player record against aim cone at a1 */
static int16_t sub_0D07E4_player(const f3mem_t *m, uint32_t obj, uint32_t a1,
                                 uint32_t player, uint16_t *out_dx, uint16_t *out_dy)
{
    if ((int16_t)rd16(m, player + 0x4Cu) < 0)
        return -1;

    uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
    int16_t d3w = (int16_t)((int16_t)rd16(m, obj + 0x18u) - (int16_t)rd16(m, player + 0x18u)
                            + (int16_t)rd16(m, a4) - 0x0F);
    int16_t a3w = d3w;
    if (d3w < 0)
        d3w = (int16_t)(-d3w);
    if ((uint16_t)d3w > rd16(m, a1 + 0x0Au))
        return -1;

    a4 -= 2u;
    int16_t d2w = (int16_t)((int16_t)rd16(m, obj + 0x14u) - (int16_t)rd16(m, player + 0x14u)
                            + (int16_t)rd16(m, a4) - 0x17);
    int16_t a4w = d2w;
    if (d2w < 0) {
        d2w = (int16_t)(-d2w);
        if ((uint16_t)d2w > rd16(m, a1 + 0x0Au))
            return -1;
        if ((uint16_t)d2w < rd16(m, a1 + 0x08u) && (uint16_t)d3w < rd16(m, a1 + 0x08u))
            return -1;
    } else {
        if ((uint16_t)d2w > rd16(m, a1 + 0x0Au))
            return -1;
        int16_t d0w = (int16_t)(rd16(m, a1 + 0x08u) - 0x1Eu);
        if (d2w < d0w && (uint16_t)d3w < rd16(m, a1 + 0x08u))
            return -1;
    }

    *out_dx = (uint16_t)d2w;
    *out_dy = (uint16_t)d3w;

    uint16_t ang = Obj_Atan256(m, a4w, a3w);
    uint16_t lo = rd16(m, a1 + 0x0Cu);
    uint16_t hi = rd16(m, a1 + 0x0Eu);
    if (lo > hi) {
        if (ang >= lo || ang <= hi)
            return (int16_t)ang;
    } else {
        if (ang >= lo && ang <= hi)
            return (int16_t)ang;
    }
    return -1;
}

/* ROM 0x0D07B4: screen-bounds check + aim cone over both players */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    uint32_t a2 = rd32(m, obj + 0x68u);
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, a2));
    *d0_io = setw(*d0_io, (uint16_t)x);
    if (x < 0x0028 || x > 0x0127)
        return;
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, a2 + 2u));
    *d0_io = setw(*d0_io, (uint16_t)y);
    if (y < 0x0010 || y > 0x00DF)
        return;

    uint16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    int16_t ang1 = sub_0D07E4_player(m, obj, a1, 0x0041B190u, &dx1, &dy1);
    int16_t ang2 = sub_0D07E4_player(m, obj, a1, 0x0041B21Au, &dx2, &dy2);

    int16_t chosen = ang2;
    *d0_io = setw(*d0_io, (uint16_t)ang2);
    if (ang1 < 0) {
        if (ang2 < 0)
            return;
    } else if (ang2 >= 0) {
        uint16_t m1 = (dx1 <= dy1) ? dx1 : dy1;
        uint16_t m2 = (dx2 <= dy2) ? dx2 : dy2;
        if (m2 >= m1)
            chosen = ang1;
    } else {
        chosen = ang1;
    }

    *d0_io = setw(*d0_io, (uint16_t)chosen);
    wr16(m, obj + 0x42u, (uint16_t)chosen);
    uint16_t chance = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    *d1_io = setw(*d1_io, chance);
    uint16_t rng = Obj_Next4Way(m);
    *d0_io = setw(*d0_io, rng);
    if ((int16_t)chance > (int16_t)rng) {
        uint16_t v = rd16(m, a1 + 0x12u);
        *d0_io = setw(*d0_io, v);
        wr8(m, obj + 0x23u, (uint8_t)v);
    }
}

/* ROM 0x0C6632 with exact D0/D1 register outputs. */
static void terrain_test_regs(const f3mem_t *m, int16_t x, int16_t y,
                              uint32_t *d0_io, uint32_t *d1_io)
{
    if (rd8(m, F3_A5 + 0x1D1Cu) != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u);
        return;
    }
    uint16_t cx = (uint16_t)((uint16_t)(x + (int16_t)rd16(m, F3_A5 + 0x0918u)) >> 3) & 0x03FFu;
    uint16_t cy = (uint16_t)((uint16_t)(y + (int16_t)rd16(m, F3_A5 + 0x091Au)) >> 3) & 0x003Fu;
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)cy * 0x0280);
    uint32_t bit = (uint32_t)cx + d1;
    int hit = Terrain_TestPoint(m, x, y);
    *d1_io = d1;
    *d0_io = (bit & 0xFFFFFF00u) | (hit ? 0xFFu : 0x00u);
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

int Fn_0F8836(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0F8836 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0F884E */
            d0 = 0x00000320u;                           /* 0x0F8918 */
            Obj_RelayTimer(m, d0);                      /* 0x0F891E */
            wr8(m, obj + 0x06u, 0u);                    /* 0x0F8924 */
            Obj_0CEF0E(m, obj);                         /* 0x0F8928 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0F885A */
        d0 = 0x00000320u;                               /* 0x0F88BE */
        Obj_0D0E04(m, obj, d0);                         /* 0x0F88C4 */
        goto death_spawn;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0F8862 */
        goto death_spawn;

    if (rd16(m, obj + 0x0Cu) != 0u)                     /* 0x0F886C */
        wr16(m, obj + 0x0Cu, (uint16_t)(rd16(m, obj + 0x0Cu) - 1u)); /* 0x0F8874 */

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F8878 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F887C */

    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F8880..0x0F8882 */
    switch ((uint16_t)d0) {                             /* 0x0F8886 -> 0x0F892E */
    case 1u: {                                          /* 0x0F8942 */
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));    /* 0x0F8942 */
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F894C */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x0F8952 */
            a1 = 0x000F8ADAu;                           /* 0x0F895A */
            if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)   /* 0x0F895E */
                return -1;
        }
        if ((int16_t)rd16(m, obj + 0x3Au) >= 0x0028) {  /* 0x0F8964 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x0F896E */
            d0 = setw(d0, (uint16_t)(Obj_Next4Way(m) & 0x001Fu)); /* 0x0F8974..0x0F897A */
            wr8(m, obj + 0x22u, (uint8_t)d0);           /* 0x0F897E */
            wr8(m, obj + 0x06u, 2u);                    /* 0x0F8982 */
        }
        break;
    }
    case 2u: {                                          /* 0x0F898A */
        if (rd16(m, obj + 0x0Cu) == 0u) {               /* 0x0F898A */
            wr8(m, obj + 0x06u, 4u);                    /* 0x0F8992 */
            break;
        }
        if (rd8(m, obj + 0x22u) != 0u) {                /* 0x0F899A */
            uint8_t c = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
            wr8(m, obj + 0x22u, c);                     /* 0x0F89A2 */
            if (c == 0u) {
                d0 = setw(d0, Obj_Next4Way(m));         /* 0x0F89AA */
                if ((int8_t)(d0 & 0xFFu) >= 0)          /* 0x0F89B0 */
                    wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) ^ 0x00B4u)); /* 0x0F89B6 */
            }
        }
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));    /* 0x0F89BC */
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F89C6 */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x0F89CC */
            a1 = 0x000F8ADAu;                           /* 0x0F89D4 */
            if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)   /* 0x0F89D8 */
                return -1;
        }
        if (rd8(m, obj + 0x22u) == 0u) {                /* 0x0F89DE */
            a1 = 0x000F8ADAu;                           /* 0x0F89E6 */
            sub_0D07B4(m, obj, a1, &d0, &d1);           /* 0x0F89EA */
            if (rd8(m, obj + 0x23u) != 0u) {            /* 0x0F89F0 */
                a1 = 0x000F8B36u;                       /* 0x0F89F8 */
                Obj_StartBehaviour(m, obj, a1);         /* 0x0F89FC */
                a1 += 8u;
                wr8(m, obj + 0x23u, 0u);                /* 0x0F8A02 */
                wr8(m, obj + 0x06u, 3u);                /* 0x0F8A06 */
                break;
            }
        }
        if (rd16(m, obj + 0x58u) != 0u) {               /* 0x0F8A0E */
            int16_t tx = (int16_t)(rd16(m, obj + 0x3Au) + 0x0010u);
            int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 0x0010u);
            terrain_test_regs(m, tx, ty, &d0, &d1);     /* 0x0F8A26 */
            if ((uint8_t)d0 != 0u)                      /* 0x0F8A2C */
                wr16(m, obj + 0x58u, 0u);               /* 0x0F8A32 */
        } else {
            if ((int16_t)rd16(m, obj + 0x3Au) > 0x0127) { /* 0x0F8A38 */
                wr16(m, obj + 0x58u, 0x00B4u);          /* 0x0F8A5E */
            } else {
                int16_t tx = (int16_t)(rd16(m, obj + 0x3Au) + 0x002Fu);
                int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 0x0010u);
                terrain_test_regs(m, tx, ty, &d0, &d1); /* 0x0F8A52 */
                if ((uint8_t)d0 != 0u)                  /* 0x0F8A58 */
                    wr16(m, obj + 0x58u, 0x00B4u);      /* 0x0F8A5E */
            }
        }
        break;
    }
    case 3u: {                                          /* 0x0F8A66 */
        d0 = rd32(m, F3_A5 + 0x4224u);                  /* 0x0F8A66 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0); /* 0x0F8A6A */
        int16_t h68 = (int16_t)rd16(m, obj + 0x68u);    /* 0x0F8A6E */
        if (h68 == 0)
            break;
        if (h68 > 0) {
            d0 = setw(d0, (uint16_t)((Obj_Next4Way(m) & 0x001Fu) + 1u)); /* 0x0F8AB8..0x0F8AC2 */
            wr8(m, obj + 0x22u, (uint8_t)d0);           /* 0x0F8AC4 */
            wr8(m, obj + 0x06u, 2u);                    /* 0x0F8AC8 */
        } else {
            d0 = Obj_Spawn(m, obj, 0x000D9992u, 9u, 2u, 0u); /* 0x0F8A8C */
            if ((int32_t)d0 >= 0) {
                a1 = d0;
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au)); /* 0x0F8A9E */
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0017u)); /* 0x0F8AA4 */
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu)); /* 0x0F8AAA */
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x000Au)); /* 0x0F8AB0 */
            }
        }
        break;
    }
    case 4u:                                            /* 0x0F8AD0 */
        d0 = rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);
        break;
    default:
        break;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F888E */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F8894 */
    Obj_FlagOffscreen(m, obj);                          /* 0x0F889A */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0F88A0 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0F890E */
        Obj_StartDying(m, obj);                         /* 0x0F8912 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x0F88AA */
    add_coll_regs(m, obj, &a1);                         /* 0x0F88B0 */
    Obj_BlinkColour(m, obj);                            /* 0x0F88B6 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_spawn:
    d0 = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);    /* 0x0F88DC */
    if ((int32_t)d0 >= 0) {
        a1 = d0;                                        /* 0x0F88EC */
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));      /* 0x0F88EE */
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0020u)); /* 0x0F88F4 */
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));      /* 0x0F88FA */
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0018u)); /* 0x0F8900 */
        wr16(m, a1 + 0x58u, 0u);                        /* 0x0F8906 */
        wr16(m, a1 + 0x60u, 0u);                        /* 0x0F890A */
    }
    wr8(m, obj + 0x06u, 0u);                            /* 0x0F890E */
    Obj_StartDying(m, obj);                             /* 0x0F8912 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
