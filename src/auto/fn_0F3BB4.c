/* ROM 0x0F3BB4: multi-part enemy sub-unit tick (A0 = object).
 * Handles bomb-clear explosion (0x0CEE9A/0x0CEEA2), HP <= 0 / screen-clear
 * death (spawning 0x0DC486 and bonus 0x117B44 when parent +0x22 hits 0),
 * scroll Y (A5+0x4228), script/motion (0x0CF1E4, 0x0CF4B8), off-screen check
 * (0x0CF85A), 3-state behaviour machine via 0x0F3DD8, script step (0x0D0690),
 * collision (0x0D0DE4), blink (0x0CBE34), sprite draw (0x001282), and aimed
 * shot spawning 0x0D9092 (0x0D09B8). */
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

/* ROM 0x0CEE52 */
static void sub_0CEE52(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u) | b;
        (void)Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, *d0_io);
    }
    Obj_StartDying(m, obj);
}

/* ROM 0x0CEE48 */
static void sub_0CEE48(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEE52(m, obj, d0_io);
}

/* ROM 0x0CEEA2 */
static void sub_0CEEA2(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u) | b;
        (void)Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, *d0_io);
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj, 0x000CEEF0u);
}

/* ROM 0x0CEE9A */
static void sub_0CEE9A(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEEA2(m, obj, d0_io);
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

/* ROM 0x0D09B8: aim at nearest live player within bounds/angle window (with D0/A1). */
static void sub_0D09B8(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_io)
{
    uint32_t hit = rd32(m, obj + 0x68u);
    int16_t sx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, hit));
    *d0_io = setw(*d0_io, (uint16_t)sx);
    if (sx < 0x28 || sx > 0x0127)
        return;
    int16_t sy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, hit + 2u));
    *d0_io = setw(*d0_io, (uint16_t)sy);
    if (sy < 0x10 || sy > 0xDF)
        return;

    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    int16_t best_ang[2] = { -1, -1 };
    uint16_t max_dx[2] = { 0u, 0u };
    uint16_t max_dy[2] = { 0u, 0u };

    uint16_t r_min = rd16(m, a1 + 0x08u);
    uint16_t r_max = rd16(m, a1 + 0x0Au);
    uint16_t a_min = rd16(m, a1 + 0x0Cu);
    uint16_t a_max = rd16(m, a1 + 0x0Eu);

    for (int i = 0; i < 2; i++) {
        uint32_t pl = players[i];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;
        int16_t sdy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u)
                                + rd16(m, hit + 2u) - 0x0F);
        uint16_t ady = (uint16_t)(sdy < 0 ? -sdy : sdy);
        if (ady > r_max)
            continue;
        int16_t sdx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u)
                                + rd16(m, hit) - 0x17);
        uint16_t adx = (uint16_t)(sdx < 0 ? -sdx : sdx);
        if (adx > r_max)
            continue;
        if (sdx < 0) {
            if (adx < r_min && ady < r_min)
                continue;
        } else {
            if ((int16_t)adx < (int16_t)(r_min - 0x1E) && ady < r_min)
                continue;
        }
        uint16_t ang = Obj_Atan256(m, sdx, sdy);
        if (a_min > a_max) {
            if (ang < a_min && ang > a_max)
                continue;
        } else {
            if (ang < a_min || ang > a_max)
                continue;
        }
        best_ang[i] = (int16_t)ang;
        max_dx[i] = adx;
        max_dy[i] = ady;
    }

    int16_t chosen = best_ang[1];
    if (best_ang[0] >= 0) {
        if (best_ang[1] < 0) {
            chosen = best_ang[0];
        } else {
            uint16_t d0 = (max_dx[0] >= max_dy[0]) ? max_dx[0] : max_dy[0];
            uint16_t d1 = (max_dx[1] >= max_dy[1]) ? max_dx[1] : max_dy[1];
            if (d1 >= d0)
                chosen = best_ang[0];
        }
    } else if (best_ang[1] < 0) {
        return;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    uint16_t thr = (uint16_t)(rd16(m, a1 + 0x10u) + rd16(m, 0x000D0D02u + (uint32_t)(rd8(m, F3_A5 + 0x087Fu) >> 3) * 2u));
    if ((int16_t)thr > (int16_t)Obj_Next4Way(m))
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

int Fn_0F3BB4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0F3BB4 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);      /* 0x0F3BBC */
        if (x >= -0x000A && x <= 0x011C) {              /* 0x0F3BC4 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0F3BCC */
            d0 = 0x000001F4u;                           /* 0x0F3D7A */
            Obj_RelayTimer(m, d0);                      /* 0x0F3D80 */
            uint32_t p = rd32(m, obj + 0x08u);          /* 0x0F3D86 */
            uint8_t cnt = (uint8_t)(rd8(m, p + 0x22u) - 1u);
            wr8(m, p + 0x22u, cnt);                     /* 0x0F3D8A */
            if (cnt != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) { /* 0x0F3D8E..0x0F3D9A */
                sub_0CEE9A(m, obj, &d0);
            } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) { /* 0x0F3DA0 */
                Obj_StartDying(m, obj);
            } else {
                d0 = 0x00001388u;                       /* 0x0F3DAC */
                Obj_RelayTimer(m, d0);                  /* 0x0F3DB2 */
                (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0); /* 0x0F3DC8 */
                sub_0CEEA2(m, obj, &d0);                /* 0x0F3DD2 */
            }
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0F3BD8 */
        d0 = 0x000001F4u;                               /* 0x0F3CC4 */
        Obj_0D0E04(m, obj, d0);                         /* 0x0F3CCA */
        goto death_spawn;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0F3BE0 */
        goto death_spawn;

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F3BEA */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F3BEE */
    {
        uint8_t f7 = rd8(m, obj + 0x07u);               /* 0x0F3BF2 */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if ((f7 & 0x02u) != 0u) {                       /* 0x0F3BF8 */
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F3BFC */
                return -1;
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {                    /* 0x0F3C02 */
        a1 = 0x000F3E5Cu;                               /* 0x0F3C0A */
        if (ObjScript_CF4B8(m, obj, a1) < 0)            /* 0x0F3C0E */
            return -1;
    }

    Obj_FlagOffscreen(m, obj);                          /* 0x0F3C14 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0F3C1A */
        uint32_t p = rd32(m, obj + 0x08u);              /* 0x0F3D66 */
        wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u)); /* 0x0F3D6A */
        wr8(m, p + 0x07u, (uint8_t)(rd8(m, p + 0x07u) | 0x01u)); /* 0x0F3D6E */
        Obj_StartDying(m, obj);                         /* 0x0F3D74 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F3C24 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F3C2A */
    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F3C30..0x0F3C32 */
    switch ((uint16_t)d0) {                             /* 0x0F3C36 -> 0x0F3DD8 */
    case 0u:                                            /* 0x0F3DE4 */
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            a1 = (rd8(m, obj + 0x22u) & 0x01u) != 0u ? 0x000F3F0Cu : 0x000F3FB4u;
            Obj_StartBehaviour(m, obj, a1);
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    case 1u:                                            /* 0x0F3E14 */
        {
            uint32_t p = rd32(m, obj + 0x08u);
            uint8_t p4e = rd8(m, p + 0x4Eu);
            d0 = (d0 & 0xFFFFFF00u) | p4e;
            if (p4e >= rd8(m, obj + 0x22u)) {
                if ((rd8(m, obj + 0x22u) & 0x01u) != 0u) {
                    Obj_StartScript(m, obj, 0x000F3EC4u);
                    a1 = 0x000F3F60u;
                } else {
                    Obj_StartScript(m, obj, 0x000F3EE8u);
                    a1 = 0x000F4008u;
                }
                Obj_StartBehaviour(m, obj, a1);
                wr8(m, obj + 0x06u, 2u);
            }
        }
        break;
    case 2u:                                            /* 0x0F3E5A */
        break;
    default:
        return -1;
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x0F3C3E */
    Obj_0D0DE4(m, obj);                                 /* 0x0F3C44 */
    Obj_BlinkColour(m, obj);                            /* 0x0F3C4A */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0F3C50 */

    if (rd32(m, obj + 0x68u) != 0u) {                   /* 0x0F3C56 */
        uint8_t cd = rd8(m, obj + 0x23u);               /* 0x0F3C5E */
        if (cd != 0u) {
            cd = (uint8_t)(cd - 1u);                    /* 0x0F3C66 */
            wr8(m, obj + 0x23u, cd);
            if (cd != 0u)
                goto done;
        }
        a1 = 0x000F3E5Cu;                               /* 0x0F3C6E */
        sub_0D09B8(m, obj, a1, &d0);                    /* 0x0F3C72 */
        if (rd8(m, obj + 0x23u) != 0u) {                /* 0x0F3C78 */
            uint32_t sp = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u); /* 0x0F3C92 */
            if ((int32_t)sp >= 0) {                     /* 0x0F3C9C */
                wr32(m, sp + 0x14u, rd32(m, obj + 0x14u)); /* 0x0F3CA4 */
                wr16(m, sp + 0x14u, (uint16_t)(rd16(m, sp + 0x14u) + 8u)); /* 0x0F3CAA */
                wr32(m, sp + 0x18u, rd32(m, obj + 0x18u)); /* 0x0F3CAE */
                wr16(m, sp + 0x18u, (uint16_t)(rd16(m, sp + 0x18u) + 8u)); /* 0x0F3CB4 */
                wr16(m, sp + 0x58u, rd16(m, obj + 0x42u)); /* 0x0F3CB8 */
                wr16(m, sp + 0x10u, 0u);                /* 0x0F3CBE */
            }
        }
    }

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_spawn:
    {
        uint32_t sp = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u); /* 0x0F3CE2 */
        if ((int32_t)sp >= 0) {                         /* 0x0F3CEC */
            wr32(m, sp + 0x14u, rd32(m, obj + 0x14u));  /* 0x0F3CF4 */
            wr16(m, sp + 0x14u, (uint16_t)(rd16(m, sp + 0x14u) + 8u)); /* 0x0F3CFA */
            wr32(m, sp + 0x18u, rd32(m, obj + 0x18u));  /* 0x0F3CFE */
            wr16(m, sp + 0x18u, (uint16_t)(rd16(m, sp + 0x18u) + 8u)); /* 0x0F3D04 */
            wr16(m, sp + 0x58u, rd16(m, obj + 0x58u));  /* 0x0F3D08 */
            wr16(m, sp + 0x60u, rd16(m, obj + 0x60u));  /* 0x0F3D0E */
        }
        uint32_t p = rd32(m, obj + 0x08u);              /* 0x0F3D14 */
        uint8_t cnt = (uint8_t)(rd8(m, p + 0x22u) - 1u);
        wr8(m, p + 0x22u, cnt);                         /* 0x0F3D18 */
        if (cnt != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) { /* 0x0F3D1C..0x0F3D28 */
            sub_0CEE48(m, obj, &d0);
        } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) { /* 0x0F3D2E */
            Obj_StartDying(m, obj);
        } else {
            d0 = 0x00001388u;                           /* 0x0F3D3A */
            Obj_0D0E04(m, obj, d0);                     /* 0x0F3D40 */
            (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0); /* 0x0F3D56 */
            sub_0CEE52(m, obj, &d0);                    /* 0x0F3D60 */
        }
    }
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
