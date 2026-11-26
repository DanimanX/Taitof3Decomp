/* ROM 0x0F95FE: walking ground-turret enemy tick (variant 2, A0 = object).
 * Same structure as 0x0F8836 with aim/behaviour tables 0x0F986C/0x0F98C8,
 * terrain probe offsets (0, +0x10) / (+0x1F, +0x10), shot 0x0D9622, and
 * death explosion offset (+0x10, +0x18). */
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
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t a2 = rd32(m, obj + 0x68u);
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, a2));
    if (x < 0x0028 || x > 0x0127)
        return;
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, a2 + 2u));
    if (y < 0x0010 || y > 0x00DF)
        return;

    uint16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    int16_t ang1 = sub_0D07E4_player(m, obj, a1, 0x0041B190u, &dx1, &dy1);
    int16_t ang2 = sub_0D07E4_player(m, obj, a1, 0x0041B21Au, &dx2, &dy2);

    int16_t chosen = ang2;
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

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    int16_t chance = (int16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    int16_t rng = (int16_t)Obj_Next4Way(m);
    if (chance > rng)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
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

int Fn_0F95FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0F95FE */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0F9616 */
            d0 = 0x00000320u;                           /* 0x0F96E0 */
            Obj_RelayTimer(m, d0);                      /* 0x0F96E6 */
            wr8(m, obj + 0x06u, 0u);                    /* 0x0F96EC */
            Obj_0CEF0E(m, obj);                         /* 0x0F96F0 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0F9622 */
        d0 = 0x00000320u;                               /* 0x0F9686 */
        Obj_0D0E04(m, obj, d0);                         /* 0x0F968C */
        goto death_spawn;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0F962A */
        goto death_spawn;

    if (rd16(m, obj + 0x0Cu) != 0u)                     /* 0x0F9634 */
        wr16(m, obj + 0x0Cu, (uint16_t)(rd16(m, obj + 0x0Cu) - 1u)); /* 0x0F963C */

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F9640 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F9644 */

    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F9648..0x0F964A */
    switch ((uint16_t)d0) {                             /* 0x0F964E -> 0x0F96F6 */
    case 1u:                                            /* 0x0F970A */
        if ((int16_t)rd16(m, obj + 0x3Au) <= 0x0127) {  /* 0x0F970A */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x0F9714 */
            wr8(m, obj + 0x06u, 2u);                    /* 0x0F971A */
        }
        d0 = rd32(m, F3_A5 + 0x4224u);                  /* 0x0F9720 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0); /* 0x0F9724 */
        break;
    case 2u: {                                          /* 0x0F972A */
        if (rd16(m, obj + 0x0Cu) == 0u) {               /* 0x0F972A */
            wr8(m, obj + 0x06u, 4u);                    /* 0x0F9732 */
            break;
        }
        if (rd8(m, obj + 0x22u) != 0u) {                /* 0x0F973A */
            uint8_t c = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
            wr8(m, obj + 0x22u, c);                     /* 0x0F9742 */
            if (c == 0u) {
                d0 = setw(d0, Obj_Next4Way(m));         /* 0x0F974A */
                if ((int8_t)(d0 & 0xFFu) >= 0)          /* 0x0F9750 */
                    wr16(m, obj + 0x58u, (uint16_t)(rd16(m, obj + 0x58u) ^ 0x00B4u)); /* 0x0F9756 */
            }
        }
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));    /* 0x0F975C */
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F9766 */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x0F976C */
            a1 = 0x000F986Cu;                           /* 0x0F9774 */
            if (ObjScript_CF4B8(m, obj, a1) < 0)        /* 0x0F9778 */
                return -1;
        }
        if (rd8(m, obj + 0x22u) == 0u) {                /* 0x0F977E */
            a1 = 0x000F986Cu;                           /* 0x0F9786 */
            sub_0D07B4(m, obj, a1);                     /* 0x0F978A */
            if (rd8(m, obj + 0x23u) != 0u) {            /* 0x0F9790 */
                a1 = 0x000F98C8u;                       /* 0x0F9798 */
                Obj_StartBehaviour(m, obj, a1);         /* 0x0F979C */
                a1 += 8u;
                wr8(m, obj + 0x23u, 0u);                /* 0x0F97A2 */
                wr8(m, obj + 0x06u, 3u);                /* 0x0F97A6 */
                break;
            }
        }
        if (rd16(m, obj + 0x58u) != 0u) {               /* 0x0F97AE */
            int16_t tx = (int16_t)rd16(m, obj + 0x3Au);
            int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 0x0010u);
            terrain_test_regs(m, tx, ty, &d0, &d1);     /* 0x0F97C2 */
            if ((uint8_t)d0 != 0u)                      /* 0x0F97C8 */
                wr16(m, obj + 0x58u, 0u);               /* 0x0F97CE */
        } else {
            if ((int16_t)rd16(m, obj + 0x3Au) > 0x0127) { /* 0x0F97D4 */
                wr16(m, obj + 0x58u, 0x00B4u);          /* 0x0F97FA */
            } else {
                int16_t tx = (int16_t)(rd16(m, obj + 0x3Au) + 0x001Fu);
                int16_t ty = (int16_t)(rd16(m, obj + 0x3Eu) + 0x0010u);
                terrain_test_regs(m, tx, ty, &d0, &d1); /* 0x0F97EE */
                if ((uint8_t)d0 != 0u)                  /* 0x0F97F4 */
                    wr16(m, obj + 0x58u, 0x00B4u);      /* 0x0F97FA */
            }
        }
        break;
    }
    case 3u: {                                          /* 0x0F9802 */
        d0 = rd32(m, F3_A5 + 0x4224u);                  /* 0x0F9802 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0); /* 0x0F9806 */
        int16_t h68 = (int16_t)rd16(m, obj + 0x68u);    /* 0x0F980A */
        if (h68 == 0)
            break;
        if (h68 > 0) {
            d0 = setw(d0, (uint16_t)((Obj_Next4Way(m) & 0x000Fu) + 1u)); /* 0x0F9854..0x0F985E */
            wr8(m, obj + 0x22u, (uint8_t)d0);           /* 0x0F9860 */
            wr8(m, obj + 0x06u, 2u);                    /* 0x0F9864 */
        } else {
            d0 = Obj_Spawn(m, obj, 0x000D9622u, 9u, 2u, 0u); /* 0x0F9828 */
            if ((int32_t)d0 >= 0) {
                a1 = d0;
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au)); /* 0x0F983A */
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0018u)); /* 0x0F9840 */
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu)); /* 0x0F9846 */
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x000Au)); /* 0x0F984C */
            }
        }
        break;
    }
    case 4u:                                            /* 0x0F9720 */
        d0 = rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);
        break;
    default:
        break;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F9656 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F965C */
    Obj_FlagOffscreen(m, obj);                          /* 0x0F9662 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0F9668 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0F96D6 */
        Obj_StartDying(m, obj);                         /* 0x0F96DA */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x0F9672 */
    add_coll_regs(m, obj, &a1);                         /* 0x0F9678 */
    Obj_BlinkColour(m, obj);                            /* 0x0F967E */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_spawn:
    d0 = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);    /* 0x0F96A4 */
    if ((int32_t)d0 >= 0) {
        a1 = d0;                                        /* 0x0F96B4 */
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));      /* 0x0F96B6 */
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u)); /* 0x0F96BC */
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));      /* 0x0F96C2 */
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0018u)); /* 0x0F96C8 */
        wr16(m, a1 + 0x58u, 0u);                        /* 0x0F96CE */
        wr16(m, a1 + 0x60u, 0u);                        /* 0x0F96D2 */
    }
    wr8(m, obj + 0x06u, 0u);                            /* 0x0F96D6 */
    Obj_StartDying(m, obj);                             /* 0x0F96DA */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
