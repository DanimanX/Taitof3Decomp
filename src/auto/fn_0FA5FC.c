/* ROM 0x0FA5FC: multi-segment articulated tentacle/arm boss-part tick (A0 = object).
 * Handles debug-kill in state 3 (0x0FA5FC..0FA624 -> 0x0FA910 -> 0x0CEF0E),
 * HP <= 0 / bomb-clear explosion chain across +0x54 linked segments (spawning
 * 0x0FB11E and 0x0FB1DE), scroll subtraction (A5+0x4224/0x4228) with left-edge
 * despawn (< -0x78), state dispatch (0x0FA928: 1=aim via 0x0D07E4, 2=extend/
 * spawn 0x0FAFE6, 3=idle), and segment chain positioning/draw/collision. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

#define REC_0FA9AE 0x000FA9AEu
#define TBL_0FA9E6 0x000FA9E6u
#define TBL_0FACC6 0x000FACC6u

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[d0w] * d1w (via 0x0D4F26). */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);
    uint32_t d1 = rd32(m, 0x000D2044u + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t idx = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)idx * 2u);
}

/* ROM 0x0D07E4 + 0x0D08A0: test one player record against aim cone at a1. */
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

    uint16_t ang = Obj_Atan256(m, a3w, a4w);
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

/* ROM 0x0D07E4: aim cone over both players (no screen-bounds precheck). */
static void sub_0D07E4(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_io)
{
    uint16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    int16_t ang1 = sub_0D07E4_player(m, obj, a1, 0x0041B190u, &dx1, &dy1);
    int16_t ang2 = sub_0D07E4_player(m, obj, a1, 0x0041B21Au, &dx2, &dy2);

    int16_t chosen = ang2;
    if (ang1 < 0) {
        if (ang2 < 0) {
            *d0_io = setw(*d0_io, (uint16_t)ang2);
            return;
        }
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
    int16_t rng = (int16_t)Rng_Mix1D60(m);
    *d0_io = setw(*d0_io, (uint16_t)rng);
    if (chance > rng) {
        uint16_t v = rd16(m, a1 + 0x12u);
        *d0_io = setw(*d0_io, v);
        wr8(m, obj + 0x23u, (uint8_t)v);
    }
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

int Fn_0FA5FC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0FA5FC: state 3 debug-kill check */
    if (rd8(m, obj + 0x06u) == 3u && (rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));        /* 0x0FA61E */
            r->d[0] = 0x0000012Cu;                            /* 0x0FA910 */
            Score_AddCurrent(m, 0x0000012Cu);                 /* 0x0FA916 */
            wr8(m, obj + 0x06u, 4u);                          /* 0x0FA91C */
            Obj_0CEF0E(m, obj);                               /* 0x0FA922 */
            return 0;
        }
    }

    /* 0x0FA62A: HP <= 0 or bomb-clear (A5-0x144B bit 5) */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        r->d[0] = 0x0000012Cu;                                /* 0x0FA77E */
        Obj_0D0E04(m, obj, 0x0000012Cu);                      /* 0x0FA784 */
        goto explode_chain;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)              /* 0x0FA632 */
        goto explode_chain;

    /* 0x0FA63C: scroll subtraction and left-edge despawn */
    {
        uint32_t dx = rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - dx);
        uint32_t dy = rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - dy);
        r->d[0] = dy;
    }
    if ((int16_t)rd16(m, obj + 0x3Au) < -0x78) {              /* 0x0FA64C */
        wr8(m, obj + 0x06u, 0u);                              /* 0x0FA906 */
        Obj_StartDying(m, obj);                               /* 0x0FA90A */
        return 0;
    }

    /* 0x0FA656: state dispatch via jump table 0x0FA928 */
    {
        uint8_t st = rd8(m, obj + 0x06u);
        r->d[0] = (r->d[0] & 0xFFFF0000u) | st;
        switch (st) {
        case 1:                                               /* 0x0FA93C */
            r->a[1] = REC_0FA9AE;
            sub_0D07E4(m, obj, REC_0FA9AE, &r->d[0]);
            if (rd8(m, obj + 0x23u) != 0u)
                wr8(m, obj + 0x06u, 2u);
            break;
        case 2:                                               /* 0x0FA956 */
            if (rd16(m, obj + 0x50u) != 9u)
                wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + 0x00004650u);
            if (rd16(m, obj + 0x08u) != 0x0Au)
                wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) + 0x0000FFFFu);
            {
                uint16_t sum = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x08u));
                r->d[0] = setw(r->d[0], sum);
                if (sum == 0x13u) {
                    wr8(m, obj + 0x06u, 3u);
                    uint32_t child = Obj_Spawn(m, obj, 0x000FAFE6u, 9u, 2u, 0u);
                    r->d[0] = child;
                }
            }
            break;
        case 3:                                               /* 0x0FA9AC: bare rts */
            break;
        default:
            break;
        }
    }

    /* 0x0FA664: traverse linked segment chain (+0x54) */
    {
        uint32_t d0 = rd32(m, obj + 0x54u);
        uint32_t cur = obj;
        for (;;) {
            cur = d0;
            r->a[0] = cur;
            if (rd8(m, cur + 0x06u) != 0u) {                  /* 0x0FA66A */
                r->d[0] = d0;
                return 0;
            }
            uint32_t parent = rd32(m, cur + 0x08u);           /* 0x0FA672 */
            wr16(m, cur + 0x60u, rd16(m, parent + 0x50u));    /* 0x0FA676 */
            if (rd16(m, cur + 0x3Eu) == 0u) {                 /* 0x0FA67C */
                wr32(m, cur + 0x14u, rd32(m, parent + 0x3Au));
                wr32(m, cur + 0x18u, rd32(m, parent + 0x3Eu));
            } else {                                          /* 0x0FA694 */
                uint32_t prev = rd32(m, cur + 0x50u);
                wr32(m, cur + 0x14u, rd32(m, prev + 0x14u));
                wr32(m, cur + 0x18u, rd32(m, prev + 0x18u));
            }

            static const uint16_t offs[3] = { 0x44u, 0x48u, 0x0Cu };
            for (int i = 0; i < 3; i++) {
                uint32_t vx = 0, vy = 0;
                uint16_t ang = rd16(m, cur + offs[i]);
                sub_0D1EBC(m, ang, rd16(m, cur + 0x60u), &vx, &vy);
                wr32(m, cur + 0x14u, rd32(m, cur + 0x14u) + vx);
                wr32(m, cur + 0x18u, rd32(m, cur + 0x18u) + vy);
                Obj_0CFA7C(m, cur, TBL_0FACC6, ang);
                draw_sprite_regs(m, cur, r);
            }

            Obj_0D0DC2(m, cur);                               /* 0x0FA720 */
            d0 = rd32(m, cur + 0x54u);                        /* 0x0FA726 */
            if ((int32_t)d0 < 0)
                break;
        }

        /* 0x0FA72E: update root object from last segment */
        uint32_t root = rd32(m, cur + 0x08u);
        wr32(m, root + 0x14u, rd32(m, cur + 0x14u));
        wr16(m, root + 0x14u, (uint16_t)(rd16(m, root + 0x14u) + rd16(m, cur + 0x68u)));
        wr32(m, root + 0x18u, rd32(m, cur + 0x18u));
        wr16(m, root + 0x18u, (uint16_t)(rd16(m, root + 0x18u) + rd16(m, cur + 0x6Au)));
        wr16(m, root + 0x58u, rd16(m, cur + 0x0Cu));

        r->a[0] = root;                                       /* 0x0FA754 */
        Obj_0D0DC2(m, root);                                  /* 0x0FA756 */
        Obj_HitFlash(m, root);                                /* 0x0FA75C */
        uint16_t dir = rd16(m, root + 0x58u);                 /* 0x0FA766 */
        r->d[0] = setw(r->d[0], dir);
        r->a[1] = TBL_0FA9E6;
        Obj_SetAnimByDir(m, root, TBL_0FA9E6, dir);           /* 0x0FA76A -> 0x0D005A */
        step_script_regs(m, root, &r->d[0], &r->a[1]);        /* 0x0FA770 -> 0x0D0690 */
        draw_sprite_regs(m, root, r);                         /* 0x0FA776 -> 0x001282 */
        return 0;
    }

explode_chain:
    /* 0x0FA78A: spawn 0x0FB11E explosions along segment chain, then 0x0FB1DE */
    {
        uint16_t d7 = 0x0030u;
        uint32_t d0 = rd32(m, obj + 0x54u);
        uint32_t cur = obj;
        for (;;) {
            cur = d0;
            r->a[0] = cur;
            if (rd16(m, cur + 0x3Eu) == 0u) {
                uint32_t parent = rd32(m, cur + 0x08u);
                wr32(m, cur + 0x14u, rd32(m, parent + 0x3Au));
                wr32(m, cur + 0x18u, rd32(m, parent + 0x3Eu));
            } else {
                uint32_t prev = rd32(m, cur + 0x50u);
                wr32(m, cur + 0x14u, rd32(m, prev + 0x14u));
                wr32(m, cur + 0x18u, rd32(m, prev + 0x18u));
            }

            static const uint16_t offs[3] = { 0x44u, 0x48u, 0x0Cu };
            for (int i = 0; i < 3; i++) {
                uint32_t vx = 0, vy = 0;
                uint16_t ang = rd16(m, cur + offs[i]);
                sub_0D1EBC(m, ang, rd16(m, cur + 0x60u), &vx, &vy);
                wr32(m, cur + 0x14u, rd32(m, cur + 0x14u) + vx);
                wr32(m, cur + 0x18u, rd32(m, cur + 0x18u) + vy);
                uint32_t child = Obj_Spawn(m, cur, 0x000FB11Eu, 9u, 2u, 0u);
                r->d[0] = child;
                if ((int32_t)child < 0)
                    goto explode_done;
                wr32(m, child + 0x14u, rd32(m, cur + 0x14u));
                wr32(m, child + 0x18u, rd32(m, cur + 0x18u));
                wr16(m, child + 0x58u, ang);
                wr8(m, child + 0x10u, (uint8_t)d7);
                d7 = (uint16_t)(d7 - 3u);
            }

            d0 = rd32(m, cur + 0x54u);
            if ((int32_t)d0 < 0)
                break;
        }

        {
            uint32_t child = Obj_Spawn(m, cur, 0x000FB1DEu, 9u, 2u, 0u);
            r->d[0] = child;
            if ((int32_t)child >= 0) {
                wr32(m, child + 0x14u, rd32(m, cur + 0x14u));
                wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + rd16(m, cur + 0x68u)));
                wr32(m, child + 0x18u, rd32(m, cur + 0x18u));
                wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + rd16(m, cur + 0x6Au)));
                wr16(m, child + 0x58u, rd16(m, cur + 0x0Cu));
            }
        }

    explode_done:
        {
            uint32_t root = rd32(m, cur + 0x08u);
            r->a[0] = root;
            wr8(m, root + 0x06u, 0u);
            Obj_StartDying(m, root);
            return 0;
        }
    }
}
