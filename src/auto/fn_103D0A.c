/* ROM 0x103D0A: boss/multi-segment articulated enemy tick (A0 = object).
 * Handles HP <= 0 transition from state 3 -> 4 (setting bit 1 of +0x34) or
 * death (awarding 0x1770 via 0x0D0E04, spawning 0x117B44 and 0x0DC5B6, clearing
 * +0x06, and calling Obj_StartDying 0x0CEE74). On bomb-active (+0x48/A5-0x144B
 * bit 5) spawns 0x0DC5B6 and starts dying. Subtracts scroll Y (A5+0x4228) from
 * +0x3E; when offscreen or bomb-inactive steps ObjScript_CF1E4 and ObjScript_CF4B8
 * with record 0x10423C. If bit 4 of +0x07 is set and (+0x3A, +0x3E) leaves
 * [-0x6E, 0x16E] x [-0x6E, 0x116], sets +0x06 = 9 and starts dying. Otherwise
 * adds to collision lists (0x0D0DC2), blinks colour (0x0CBE34), syncs
 * (+0x14, +0x18) from (+0x3A, +0x3E), dispatches state +0x06 via table
 * 0x104006 (states 1..8), runs 3D-orbit offset + homing (0x0CF898, 0x0D0E52,
 * 0x0D2D44) when onscreen with bomb active, updates and draws the linked
 * articulated segment chain (+0x54) unless state is 1, 5, or 6, and finally
 * draws the head sprite (0x001282) and restores +0x14 = +0x3A. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
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

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[d0w] * d1w (via 0x0D4F26). */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    *out_d0 = rd32(m, 0x000D1EDCu + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2044u + off) * (uint32_t)d1w;
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    *out_d0 = rd32(m, 0x000D2D64u + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2ECCu + off) * (uint32_t)d1w;
}

/* ROM 0x0D4A2C: swapped high-word scaled values for k0, k1 <= 50. */
static void sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t tbl = rd32(m, 0x000D4A5Au + (uint32_t)(int32_t)(int16_t)d2w * 4u);
    uint32_t p1 = tbl * (uint32_t)d1w;
    uint32_t p0 = tbl * (uint32_t)d0w;
    *out_d1 = (p1 >> 16) | (p1 << 16);
    *out_d0 = (p0 >> 16) | (p0 << 16);
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

static int is_onscreen_bomb(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        return 0;
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    return x >= -0x000A && x <= 0x011C;
}

/* ROM 0x104070 (dir=+1) and 0x1040C2 (dir=-1): sweep +0x1F and draw dual sprite. */
static void state_sweep(const f3mem_t *m, uint32_t obj, int dir,
                        uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint8_t ang;
    uint16_t k0, k1;
    uint32_t end_spr;

    if (dir > 0) {
        ang = (uint8_t)(rd8(m, obj + 0x1Fu) + 0x0Fu);
        wr8(m, obj + 0x1Fu, ang);
        if (ang == 0xB4u)
            wr8(m, obj + 0x06u, 6u);
        k0 = 0x28u;
        k1 = 0x20u;
        end_spr = 0x0002BDE8u;
    } else {
        ang = (uint8_t)(rd8(m, obj + 0x1Fu) - 0x0Fu);
        wr8(m, obj + 0x1Fu, ang);
        if (ang == 0u)
            wr8(m, obj + 0x06u, 7u);
        k0 = 0x20u;
        k1 = 0x28u;
        end_spr = 0x0002BE2Cu;
    }

    uint16_t d2w = (uint8_t)(~ang);
    uint32_t d0, d1;
    sub_0D4A2C(m, k0, k1, d2w, &d0, &d1);
    *d0_io = d0;
    *d1_io = d1;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
    uint32_t a4 = d1;
    wr32(m, obj + 0x10u, 0x0002BE70u);
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
    *d0_io = setw(*d0_io, (uint16_t)a4);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)a4));
    wr32(m, obj + 0x10u, end_spr);
}

/* ROM 0x1041D0 (state 2, next=3, set bit4 of +0x07) and 0x10410E (state 7, next=8). */
static void state_expand(const f3mem_t *m, uint32_t obj, uint32_t spr,
                         uint8_t next_state, int set_offscreen_bit,
                         uint32_t *d0_io)
{
    wr32(m, obj + 0x10u, spr);
    if (rd16(m, obj + 0x48u) != 7u)
        wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x00005208u);
    if (rd16(m, obj + 0x08u) != 0x000Au)
        wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) + 0x0000FFFFu);
    uint16_t sum = (uint16_t)(rd16(m, obj + 0x48u) + rd16(m, obj + 0x08u));
    *d0_io = setw(*d0_io, sum);
    if (sum == 0x0011u) {
        wr32(m, obj + 0x24u, 0x000F0000u);
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
        if (set_offscreen_bit)
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
        wr8(m, obj + 0x06u, next_state);
    }
}

/* ROM 0x104006 dispatch for states 1..8. */
static int run_state(const f3mem_t *m, uint32_t obj, uint8_t state,
                     uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    switch (state) {
    case 1u: /* 0x104226 */
    case 3u: /* 0x1041CE */
        return 0;

    case 2u: /* 0x1041D0 */
        state_expand(m, obj, 0x0002BD60u, 3u, 1, d0_io);
        return 0;

    case 4u: { /* 0x10402E */
        if (rd16(m, obj + 0x48u) != 1u)
            wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) - 0x00005208u);
        if (rd16(m, obj + 0x08u) != 0u)
            wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) - 0x0000FFFFu);
        uint16_t sum = (uint16_t)(rd16(m, obj + 0x48u) + rd16(m, obj + 0x08u));
        *d0_io = setw(*d0_io, sum);
        if (sum == 1u) {
            wr32(m, obj + 0x10u, 0x0002BDE8u);
            wr8(m, obj + 0x06u, 5u);
        }
        return 0;
    }

    case 5u: /* 0x104070 */
        state_sweep(m, obj, 1, d0_io, d1_io, a1_io);
        return 0;

    case 6u: /* 0x1040C2 */
        state_sweep(m, obj, -1, d0_io, d1_io, a1_io);
        return 0;

    case 7u: /* 0x10410E */
        state_expand(m, obj, 0x0002BDA4u, 8u, 0, d0_io);
        return 0;

    case 8u: { /* 0x104160 */
        uint8_t cd = rd8(m, obj + 0x23u);
        if (cd != 0u) {
            cd = (uint8_t)(cd - 1u);
            wr8(m, obj + 0x23u, cd);
            if (cd != 0u)
                return 0;
        }
        *a1_io = 0x0010423Cu;
        Obj_AimAtPlayers(m, obj, 0x0010423Cu);
        if (rd8(m, obj + 0x23u) != 0u) {
            uint32_t child = Obj_Spawn(m, obj, 0x000DB074u, 9u, 2u, 0u);
            *d0_io = child;
            if ((int32_t)child >= 0) {
                *a1_io = child;
                wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0020u));
                wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0021u));
                wr16(m, child + 0x58u, rd16(m, obj + 0x42u));
                wr16(m, child + 0x10u, 0u);
                wr8(m, obj + 0x1Du, 0x70u);
            }
        }
        return 0;
    }

    default:
        return -1;
    }
}

/* ROM 0x103E02..0x103F1E: position and draw articulated arm segments. */
static void update_segments(const f3mem_t *m, uint32_t *a0_io,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t d0 = rd32(m, *a0_io + 0x54u);
    uint32_t seg;

    do {
        seg = d0;
        *a0_io = seg;
        if (rd8(m, seg + 0x06u) == 0u) {
            uint32_t head = rd32(m, seg + 0x08u);
            uint8_t hstate = rd8(m, head + 0x06u);
            wr8(m, seg + 0x4Eu, hstate);
            wr16(m, seg + 0x60u, rd16(m, head + 0x48u));

            uint32_t a2 = (hstate <= 5u) ? 0x001042ECu : 0x001042F8u;
            int16_t idx3e = (int16_t)rd16(m, seg + 0x3Eu);
            uint32_t a4 = rd32(m, a2 + (uint32_t)((int32_t)idx3e * 4));

            if (idx3e == 0) {
                wr32(m, seg + 0x14u, rd32(m, head + 0x3Au));
                wr32(m, seg + 0x18u, rd32(m, head + 0x18u));
                int16_t idx3a = (int16_t)rd16(m, seg + 0x3Au);
                uint32_t p = 0x00104228u + (uint32_t)((int32_t)idx3a * 4);
                wr16(m, seg + 0x14u, (uint16_t)(rd16(m, seg + 0x14u) + rd16(m, p)));
                wr16(m, seg + 0x18u, (uint16_t)(rd16(m, seg + 0x18u) + rd16(m, p + 2u)));
            } else {
                uint32_t prev = rd32(m, seg + 0x50u);
                wr32(m, seg + 0x14u, rd32(m, prev + 0x14u));
                wr32(m, seg + 0x18u, rd32(m, prev + 0x18u));
            }

            static const uint8_t ang_offs[3] = { 0x44u, 0x48u, 0x0Cu };
            uint16_t scale = rd16(m, seg + 0x60u);
            for (int i = 0; i < 3; i++) {
                uint16_t ang = rd16(m, seg + ang_offs[i]);
                uint32_t vx, vy;
                sub_0D1EBC(m, ang, scale, &vx, &vy);
                wr32(m, seg + 0x14u, rd32(m, seg + 0x14u) + vx);
                wr32(m, seg + 0x18u, rd32(m, seg + 0x18u) + vy);
                uint32_t tbl = rd32(m, a4);
                if (i < 2)
                    a4 += 4u;
                Obj_0CFA7C(m, seg, tbl, ang);
                draw_sprite_regs(m, seg, d0_io, d1_io, a1_io);
            }

            if (hstate != 2u && hstate != 4u && hstate != 7u)
                add_coll_regs(m, seg, a1_io);
        }
        d0 = rd32(m, seg + 0x54u);
        *d0_io = d0;
    } while ((int32_t)d0 >= 0);

    *a0_io = rd32(m, seg + 0x08u);
}

int Fn_103D0A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, obj + 0x34u) & 0x02u) == 0u &&          /* 0x103D0A */
        (int32_t)rd32(m, obj + 0x24u) <= 0) {
        if (rd8(m, obj + 0x06u) == 3u) {                /* 0x103F68 */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
            wr8(m, obj + 0x06u, 4u);
            goto post_move;
        }
        d0 = 0x00001770u;                               /* 0x103F82 */
        {
            uint8_t mode = rd8(m, obj + 0x35u);
            if (mode == 1u)
                a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
            else if (mode == 2u)
                a1 = rd32(m, obj + 0x36u);
        }
        Obj_0D0E04(m, obj, d0);
        Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d0);
        goto bomb_death;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x103D1C */
        goto bomb_death;

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x103D26 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);

    if (!is_onscreen_bomb(m, obj)) {                   /* 0x103D2E */
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x103D56 */
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {                /* 0x103D5C */
            a1 = 0x0010423Cu;
            if (cf4b8_regs(m, obj, a1, &d0, &d1) < 0)
                return -1;
        }
    }

    if ((rd8(m, obj + 0x07u) & 0x10u) != 0u) {          /* 0x103D6E */
        int16_t x3a = (int16_t)rd16(m, obj + 0x3Au);
        int16_t y3e = (int16_t)rd16(m, obj + 0x3Eu);
        if (x3a < -0x006E || x3a > 0x016E ||
            y3e < -0x006E || y3e > 0x0116) {
            wr8(m, obj + 0x06u, 9u);                    /* 0x103FFA */
            Obj_StartDying(m, obj);
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    add_coll_regs(m, obj, &a1);                         /* 0x103DA0 */
    Obj_BlinkColour(m, obj);                            /* 0x103DA6 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x103DAC */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x103DB2 */

    {
        uint8_t state = rd8(m, obj + 0x06u);
        d0 = (d0 & 0xFFFF0000u) | state;
        if (run_state(m, obj, state, &d0, &d1, &a1) < 0)
            return -1;
    }

    if (is_onscreen_bomb(m, obj)) {                    /* 0x103DC6 */
        a1 = rd32(m, obj + 0x0Cu);
        if (Obj_0CF898(m, obj, &d0, &d1) < 0)           /* 0x103F30 */
            return -1;
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
        int16_t dx = (int16_t)(rd16(m, obj + 0x3Au) - 0x0081);
        int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - 0x0055);
        uint16_t ang = Obj_Atan256(m, dx, dy);          /* 0x103F4E */
        sub_0D2D44(m, ang, 7u, &d0, &d1);               /* 0x103F56 */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);
    }

post_move:
    {
        uint8_t st = rd8(m, obj + 0x06u);
        if (st != 1u && st != 5u && st != 6u)
            update_segments(m, &obj, &d0, &d1, &a1);
    }

    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x103F22 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x103F28 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[0] = obj;
    r->a[1] = a1;
    return 0;

bomb_death:
    d0 = Obj_Spawn(m, obj, 0x000DC5B6u, 3u, 2u, 0u);   /* 0x103FA8 */
    if ((int32_t)d0 >= 0) {
        a1 = d0;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x001Fu));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x001Fu));
        wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
    }
    wr8(m, obj + 0x06u, 0u);                            /* 0x103FF0 */
    Obj_StartDying(m, obj);                             /* 0x103FF4 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
