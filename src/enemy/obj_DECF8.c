#include "obj_DECF8.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/*
 * ROM 0x0DECF8: projectile tick. A0 = obj, A1 = obj+0x0C (its parent).
 * Object fields: +0x06 state.b, +0x08 angle.w, +0x0C parent, +0x10 sprite def,
 * +0x14 x, +0x18 y, +0x1C flip.b, +0x23 timer.b, +0x24 hp, +0x42 aim angle,
 * +0x48 table cur, +0x4C table count, +0x68 selector.
 */

#define O_STATE      0x06u
#define O_ANGLE_IN   0x08u
#define O_CURSOR     0x0Cu
#define O_SPRDEF     0x10u
#define O_X          0x14u
#define O_Y          0x18u
#define O_FLIP       0x1Cu
#define O_COLOR      0x1Du
#define O_EASE       0x1Eu
#define O_EASE1      0x1Fu
#define O_TIMER      0x23u
#define O_HP         0x24u
#define O_ANGLE      0x2Cu
#define O_RADIUS     0x2Eu
#define O_ZOOM       0x30u
#define O_INV        0x32u
#define O_VISIBLE    0x34u
#define O_FLASH_TGL  0x35u
#define O_OWNER      0x36u
#define O_ANGLE_OUT  0x42u
#define O_TBL_CUR    0x48u
#define O_TBL_CNT    0x4Cu
#define O_COLOR_SRC  0x4Fu
#define O_SEL        0x68u

#define G_SCROLL_Y    0x4062A4u  /* A5-0x1D5C */
#define G_FLASH_EN    0x409D53u  /* A5+0x1D53 */
#define G_STAGE_FLAGS 0x409D58u  /* A5+0x1D58 */
#define G_AIM_TOL     0x40887Fu  /* A5+0x087F */
#define G_HAZ_CURSOR  0x4088FEu  /* A5+0x08FE */
#define G_RAND_W      0x409D60u  /* A5+0x1D60 */

#define PL0_STATE P1_STATE_ADDR
#define PL1_STATE (P1_STATE_ADDR + 0x8Au)

#define W(v)   ((int32_t)(int16_t)(uint16_t)(v))
#define U16(v) ((uint16_t)(v))
#define S32(v) ((int32_t)(uint32_t)(v))

static uint16_t ld16(const f3mem_t *m, uint32_t a) { return rd16(m, a); }
static uint32_t ld32(const f3mem_t *m, uint32_t a) { return rd32(m, a); }
static uint8_t  ld8(const f3mem_t *m, uint32_t a) { return rd8(m, a); }

static void sub_0D0678(const f3mem_t *m, uint32_t a0, uint32_t a1);
static void sub_0D0690(const f3mem_t *m, uint32_t a0);
static void sub_0CFA7C(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t d0w);
static void sub_0D07B4(const f3mem_t *m, uint32_t a0, uint32_t a1);
static void sub_0D0BBC(const f3mem_t *m, uint32_t a0, uint32_t a1);
static int  sub_0DEED8(const f3mem_t *m, uint32_t a0, uint32_t a1);
static int  sub_0DEFC0(const f3mem_t *m, uint32_t a0, uint32_t a1);
static int  lab_0DEE94(const f3mem_t *m, uint32_t a0, int16_t d0w);

/* ROM 0x00649A: D0.w = angle index from the doubled squared distance. */

/* ROM 0x006422: polar table lookup. */

/* ROM 0x0CBB2C: orbiter re-arm body at 0x0CBB50. */

/* ROM 0x0CBE34 */

/* ROM 0x0CEF0E */
static void sub_0CEF0E(const f3mem_t *m, uint32_t a0)
{
    if ((ld8(m, G_FLASH_EN) & 0x20u) != 0u) {
        Obj_StartDying(m, a0);
        return;
    }
    if ((ld8(m, G_STAGE_FLAGS) & 0x02u) != 0u) {
        int32_t x = W(ld16(m, a0 + O_X));
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, a0 + O_COLOR, ld8(m, a0 + O_COLOR_SRC));
            Obj_ExplodeTick(m, a0);
            return;
        }
    }
    wr32(m, a0 + 0x00u, 0x000CEF44u);
}

/* ROM 0x0D0678 */
static void sub_0D0678(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    wr32(m, a0 + O_TBL_CUR, a1);
    wr16(m, a0 + O_TBL_CNT, ld16(m, a1));
    wr8(m, a0 + O_FLIP, (uint8_t)ld16(m, a1 + 2u));
    wr32(m, a0 + O_SPRDEF, ld32(m, a1 + 4u));
    wr32(m, a0 + O_SEL, ld32(m, a1 + 8u));
}

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t a0)
{
    uint16_t c = (uint16_t)(ld16(m, a0 + O_TBL_CNT) - 1u);
    uint32_t a1;
    uint16_t d0;

    wr16(m, a0 + O_TBL_CNT, c);
    if (c != 0u) {
        return;
    }
    a1 = ld32(m, a0 + O_TBL_CUR) + 0xCu;
    wr32(m, a0 + O_TBL_CUR, a1);
    d0 = ld16(m, a1);
    a1 += 2u;
    if (W(d0) >= 0) {
        wr16(m, a0 + O_TBL_CNT, d0);
        wr8(m, a0 + O_FLIP, (uint8_t)ld16(m, a1));
        wr32(m, a0 + O_SPRDEF, ld32(m, a1 + 2u));
        wr32(m, a0 + O_SEL, ld32(m, a1 + 6u));
        return;
    }
    if (ld16(m, a1) == 0u) {
        return;
    }
    a1 -= ld16(m, a1);
    wr32(m, a0 + O_TBL_CUR, a1);
    wr16(m, a0 + O_TBL_CNT, ld16(m, a1));
    wr8(m, a0 + O_FLIP, (uint8_t)ld16(m, a1 + 2u));
    wr32(m, a0 + O_SPRDEF, ld32(m, a1 + 4u));
    wr32(m, a0 + O_SEL, ld32(m, a1 + 8u));
}

/* ROM 0x0D0DC2: append the object to the hazard list. */
static void sub_0D0DC2(const f3mem_t *m, uint32_t a0)
{
    uint32_t a1 = ld32(m, G_HAZ_CURSOR);
    wr32(m, a1, a0);
    wr32(m, G_HAZ_CURSOR, a1 + 4u);
    Coll_AddHazard(m, a0);                 /* falls through to 0xD0DCC */
}

/* ROM 0x0D0E04: score through the owner chain. */
static void sub_0D0E04(const f3mem_t *m, uint32_t a0, uint32_t d0)
{
    uint32_t a1;

    if (ld8(m, a0 + O_FLASH_TGL) == 1u) {
        a1 = ld32(m, a0 + O_OWNER);
        a1 = ld32(m, a1 + 8u);
        wr32(m, a1 + 0x48u, ld32(m, a1 + 0x48u) + d0);
        return;
    }
    if (ld8(m, a0 + O_FLASH_TGL) == 2u) {
        a1 = ld32(m, a0 + O_OWNER);
        wr32(m, a1 + 0x48u, ld32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0BBC: turn obj+0x08 toward obj+0x42. */
static void sub_0D0BBC(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    uint16_t d0 = ld16(m, a0 + O_ANGLE_OUT);
    uint16_t d1 = d0;
    uint16_t rate = ld16(m, a1 + 4u);

    d0 = (uint16_t)(d0 - ld16(m, a0 + O_ANGLE_IN));
    if (W(d0) < 0) {
        d0 = (uint16_t)(d0 + 0x168u);
    }
    if (d0 > 0xB4u) {
        d0 = (uint16_t)(U16(0u - d0) + 0x168u);
        if (d0 > rate) {
            wr32(m, a0 + O_ANGLE_IN, ld32(m, a0 + O_ANGLE_IN) - ld32(m, a1 + 4u));
            if (S32(ld32(m, a0 + O_ANGLE_IN)) < 0) {
                wr16(m, a0 + O_ANGLE_IN, U16(ld16(m, a0 + O_ANGLE_IN) + 0x168u));
            }
            return;
        }
    } else if (d0 > rate) {
        wr32(m, a0 + O_ANGLE_IN, ld32(m, a0 + O_ANGLE_IN) + ld32(m, a1 + 4u));
        if (ld16(m, a0 + O_ANGLE_IN) >= 0x168u) {
            wr16(m, a0 + O_ANGLE_IN, U16(ld16(m, a0 + O_ANGLE_IN) - 0x168u));
        }
        return;
    }
    wr16(m, a0 + O_ANGLE_IN, d1);
}

/* ROM 0x0D0C8C */
static int16_t sub_0D0C8C(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(ld8(m, G_AIM_TOL) >> 3);
    return (int16_t)ld16(m, (0xD0D02u + (uint32_t)(uint16_t)(d0 << 1)));
}

/* ROM 0x0D0E3A: PRNG step at A5+0x1D60, high byte returned. */

/* bfffo on a word register, 68020 MSB-first numbering. */
static uint16_t bfffo_w(uint16_t v)
{
    uint32_t i;
    for (i = 0u; i < 16u; i++) {
        if ((v & (uint16_t)(0x8000u >> i)) != 0u) {
            return (uint16_t)(0x10u + i);
        }
    }
    return 0x20u;
}

/* ROM 0x0D0E52: octant angle of (D0.w, D1.w). */
static uint16_t sub_0D0E52(const f3mem_t *m, int16_t d0w, int16_t d1w)
{
    uint16_t d2 = 0u;
    uint16_t u0 = (uint16_t)d0w;
    uint16_t u1 = (uint16_t)d1w;
    int16_t d3;
    uint16_t r;

    if (W(u0) < 0) { d2 = (uint16_t)(d2 + 2u); u0 = (uint16_t)(0u - u0); }
    if (W(u1) < 0) { d2 = (uint16_t)(d2 + 4u); u1 = (uint16_t)(0u - u1); }
    d3 = (int16_t)((u1 > u0) ? bfffo_w(u1) : bfffo_w(u0));
    d3 = (int16_t)((uint16_t)d3 - 0x1Au);
    if (d3 < 0) {
        uint32_t sh = (uint32_t)(-(int32_t)d3) & 15u;
        u0 = (uint16_t)(u0 >> sh);
        u1 = (uint16_t)(u1 >> sh);
    }
    if (d2 != 2u && d2 != 4u) {
        uint16_t t = u0; u0 = u1; u1 = t;
    }
    r = (uint16_t)((uint16_t)(u1 << 6) + u0);
    r = (uint16_t)(ld8(m, (0xD0EBCu + r))
                   + ld16(m, (0xD0EB4u + (uint32_t)d2)));
    return (r == 0x168u) ? 0u : r;
}

/* Window test of the signed (ex, ey) offset against {lo, hi} at A1+8. */
static int32_t aim_admit(const f3mem_t *m, uint32_t a1, int16_t ex, int16_t ey)
{
    uint16_t d0 = sub_0D0E52(m, ex, ey);
    uint16_t lo = ld16(m, (a1 + 0x8u));
    uint16_t hi = ld16(m, (a1 + 0xAu));

    if (d0 > hi) {
        return -1;
    }
    if (lo <= hi) {
        return (d0 < lo) ? -1 : (int32_t)d0;
    }
    if (d0 < lo) {
        return (int32_t)d0;
    }
    return (d0 > hi) ? (int32_t)d0 : -1;
}

/* Per-player cone test of 0x0D07E4 / 0x0D08A0; returns the admitted angle or
 * -1 when the offset falls outside the cone. */
static int32_t cone_test(const f3mem_t *m, uint32_t a0, uint32_t a1, uint32_t pl,
                         int32_t *out_ex, int32_t *out_ey)
{
    uint16_t dy = (uint16_t)((uint16_t)(ld16(m, a0 + O_Y) - ld16(m, pl + 0x18u))
                             + ld16(m, a1 + 2u) - 0x0Fu);
    uint16_t dx;
    uint16_t tol = ld16(m, (a1 + 0xAu));
    uint16_t thr = ld16(m, (a1 + 0x8u));

    *out_ey = W(dy);
    *out_ex = 0;
    if (U16((W(dy) < 0) ? 0u - dy : dy) > tol) {
        return -1;
    }
    dx = (uint16_t)((uint16_t)(ld16(m, a0 + O_X) - ld16(m, pl + 0x14u))
                    + ld16(m, a1) - 0x17u);
    *out_ex = W(dx);
    if (W(dx) < 0) {
        if (U16(0u - dx) > tol) {
            return -1;
        }
        if (W(dy) >= 0 && U16(0u - dx) >= thr && W(dy) >= (int32_t)thr) {
            return -1;
        }
        return aim_admit(m, a1, *out_ex, *out_ey);
    }
    if (dx > tol) {
        return -1;
    }
    if (dx < (uint16_t)(thr - 0x1Eu) && W(dy) < (int32_t)thr) {
        return -1;
    }
    return aim_admit(m, a1, *out_ex, *out_ey);
}

/* ROM 0x0D07B4: aim cone over both player records. */
static void sub_0D07B4(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    uint16_t bx = (uint16_t)(ld16(m, a0 + O_X) + ld16(m, a1));
    uint16_t by;
    int32_t d4 = -1;
    int32_t d7 = -1;
    int32_t ex0 = 0, ey0 = 0, ex1 = 0, ey1 = 0;

    if (W(bx) < 0x28 || W(bx) > 0x127) {
        return;
    }
    by = (uint16_t)(ld16(m, a0 + O_Y) + ld16(m, a1 + 2u));
    if (W(by) < 0x10 || W(by) > 0xDF) {
        return;
    }
    if (W(ld16(m, PL0_STATE + 0x4Cu)) >= 0) {
        d4 = cone_test(m, a0, a1, PL0_STATE, &ex0, &ey0);
    }
    if (W(ld16(m, PL1_STATE + 0x4Cu)) >= 0) {
        d7 = cone_test(m, a0, a1, PL1_STATE, &ex1, &ey1);
    }
    {
        int32_t d0 = 0;

        if (d4 < 0) {
            if (d7 < 0) {
                return;
            }
            d0 = d7;
        } else if (d7 < 0) {
            d0 = d4;
        } else {
            int32_t d2 = (ex0 > ey0) ? ex0 : ey0;
            int32_t d5 = (ex1 > ey1) ? ex1 : ey1;

            d0 = (d5 < d2) ? d4 : d7;
        }
        wr16(m, a0 + O_ANGLE_OUT, (uint16_t)d0);
    }
    {
        int16_t thresh = (int16_t)(uint16_t)(ld16(m, a1 + 0x10u) + (uint16_t)sub_0D0C8C(m));
        uint16_t r = Obj_Next4Way(m);

        if (W(r) > thresh) {
            wr8(m, a0 + O_TIMER, (uint8_t)ld16(m, a1 + 0x12u));
        }
    }
}

/* ROM 0x0DEE10: state 1, flight / seek. */
/* ROM 0x0D0090: A1 += table16[0xD00B4 + 2*D0.w] (sign-extended), A1 = *A1,
 * then start that animation sequence (as 0xD0678). */
static void sub_0D0090(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t d0)
{
    int16_t off = (int16_t)ld16(m, 0x000D00B4u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    a1 = ld32(m, a1 + (uint32_t)(int32_t)off);
    wr32(m, a0 + 0x48u, a1);
    wr16(m, a0 + 0x4Cu, ld16(m, a1));
    wr8(m, a0 + 0x1Cu, (uint8_t)ld16(m, a1 + 2u));
    wr32(m, a0 + 0x10u, ld32(m, a1 + 4u));
    wr32(m, a0 + 0x68u, ld32(m, a1 + 8u));
}

static int sub_0DEE10(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    uint16_t d2;

    if (ld8(m, a0 + O_TIMER) != 0u) {
        goto lab_0DEE76;
    }
    if ((ld8(m, (a1 + 0x07u)) & 0x20u) == 0u) {
        return 0;
    }
    sub_0D07B4(m, a0, 0x0DEFE6u);
    d2 = ld16(m, a0 + O_ANGLE_OUT);
    if (d2 >= 0x50u && d2 <= 0x64u) {
        wr8(m, a0 + O_TIMER, 0u);
        return 0;
    }
    sub_0D0BBC(m, a0, a1);
    d2 = (uint16_t)((uint16_t)(ld16(m, a0 + O_ANGLE_OUT) - 0x5Au)
                    ^ (uint16_t)(ld16(m, a0 + O_ANGLE_IN) - 0x5Au));
    if (W(d2) < 0) {
        return lab_0DEE94(m, a0, (int16_t)d2);
    }
    sub_0CFA7C(m, a0, 0x0DF01Eu, ld16(m, a0 + O_ANGLE_IN));
    if (ld8(m, a0 + O_TIMER) == 0u) {
        return 0;
    }
lab_0DEE76:
    {
        uint8_t c = (uint8_t)(ld8(m, a0 + O_TIMER) - 1u);
        wr8(m, a0 + O_TIMER, c);
        if (c != 0u) {
            return 0;
        }
    }
    sub_0D0090(m, a0, 0x000DF0BEu, ld16(m, a0 + 0x08u));  /* lea (0x23e,PC) at 0xDEE7E */
    wr8(m, a0 + O_STATE, 2u);
    return 0;
}

/* ROM 0x0DEE94: aim rejected; pick the table for the aim side. */
static int lab_0DEE94(const f3mem_t *m, uint32_t a0, int16_t d0w)
{
    wr8(m, a0 + O_TIMER, 0u);
    wr8(m, a0 + O_STATE, 4u);
    if (d0w < 0) {
        sub_0D0678(m, a0, 0x0DF4A6u);
        wr16(m, a0 + O_ANGLE_OUT, 0x86u);
        wr32(m, a0 + O_ANGLE_IN, 0x00860000u);
        return 0;
    }
    sub_0D0678(m, a0, 0x0DF4EEu);
    wr16(m, a0 + O_ANGLE_OUT, 0x2Eu);
    wr32(m, a0 + O_ANGLE_IN, 0x002E0000u);
    return 0;
}

/* ROM 0x0DEED8: state 2, fire the burst child once the table ends. */
static int sub_0DEED8(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    int16_t d1;
    uint32_t child;

    sub_0D0690(m, a0);
    if (ld16(m, a0 + O_TBL_CNT) == 0u) {
        sub_0CFA7C(m, a0, 0x0DF01Eu, ld16(m, a0 + O_ANGLE_IN));
        wr8(m, a0 + O_TIMER, 5u);
        wr8(m, a0 + O_STATE, 3u);
        return 0;
    }
    d1 = W(ld16(m, (a0 + O_SEL)));
    if (d1 < 0) {
        return 0;
    }
    if (d1 == 0) {
        uint32_t par = ld32(m, a0 + O_CURSOR);
        wr8(m, par + 0x06u, 2u);
        return 0;
    }
    child = Obj_Spawn(m, a0, 0x00DC328u, 2u, 9u, 0x000D9092u);
    if (S32(child) < 0) {
        return 0;
    }
    wr32(m, child + O_X, ld32(m, a0 + O_X));
    wr16(m, child + O_X, U16(ld16(m, child + O_X) + 0x20u));
    wr32(m, child + O_Y, ld32(m, a0 + O_Y));
    wr16(m, child + O_Y, U16(ld16(m, child + O_Y) + 0x1Eu));
    wr16(m, child + 0x58u, (uint16_t)d1);
    wr16(m, child + 0x60u, 0x0Fu);
    (void)a1;
    return 0;
}

/* ROM 0x0DEF62: state 3, active burst. */
static int sub_0DEF62(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    uint16_t d2;
    uint8_t tmp;

    {
        uint8_t c = (uint8_t)(ld8(m, a0 + O_TIMER) - 1u);
        wr8(m, a0 + O_TIMER, c);
        if (c == 0u) {
            wr8(m, a0 + O_STATE, 1u);
            return 0;
        }
    }
    tmp = ld8(m, a0 + O_TIMER);
    sub_0D07B4(m, a0, 0x0DEFE6u);
    wr8(m, a0 + O_TIMER, tmp);
    d2 = ld16(m, a0 + O_ANGLE_OUT);
    if (d2 >= 0x50u && d2 <= 0x64u) {
        return 0;
    }
    sub_0D0BBC(m, a0, a1);
    d2 = (uint16_t)((uint16_t)(ld16(m, a0 + O_ANGLE_OUT) - 0x5Au)
                    ^ (uint16_t)(ld16(m, a0 + O_ANGLE_IN) - 0x5Au));
    if (W(d2) < 0) {
        return lab_0DEE94(m, a0, (int16_t)d2);
    }
    sub_0CFA7C(m, a0, 0x0DF01Eu, ld16(m, a0 + O_ANGLE_IN));
    return 0;
}

/* ROM 0x0DEFC0: state 4, table animation. */
static int sub_0DEFC0(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    sub_0D0690(m, a0);
    if (ld16(m, a0 + O_TBL_CNT) != 0u) {
        return 0;
    }
    sub_0CFA7C(m, a0, 0x0DF01Eu, ld16(m, a0 + O_ANGLE_IN));
    wr8(m, a0 + O_STATE, 1u);
    (void)a1;
    return 0;
}

/* ROM 0x0CFA7C */
static void sub_0CFA7C(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t d0w)
{
    /* move.w (0xcfa96,PC,D0w*2),D0w ; adda.w D0w,A1 ; then 0xCFA86 */
    int16_t off = (int16_t)ld16(m, 0x000CFA96u + (uint32_t)(int32_t)(int16_t)d0w * 2u);
    a1 += (uint32_t)(int32_t)off;
    wr8(m, a0 + O_FLIP, (uint8_t)ld16(m, a1));
    wr32(m, a0 + O_SPRDEF, ld32(m, a1 + 2u));
    wr32(m, a0 + O_SEL, ld32(m, a1 + 6u));
}

/* Copy the parent position, then add the word deltas. */
static void follow_parent(const f3mem_t *m, uint32_t a0, uint32_t par, int32_t dx, int32_t dy)
{
    wr32(m, a0 + O_X, ld32(m, par + O_X));
    wr16(m, a0 + O_X, U16(ld16(m, a0 + O_X) + (uint16_t)dx));
    wr32(m, a0 + O_Y, ld32(m, par + O_Y));
    wr16(m, a0 + O_Y, U16(ld16(m, a0 + O_Y) + (uint16_t)dy));
}

/* ROM 0x0DECF8 */
int Obj_DECF8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = ld32(m, obj + O_CURSOR);
    int rc = 0;

    if (ld8(m, a1 + O_STATE) == 0u) {
        wr8(m, obj + O_STATE, 0u);
        sub_0CEF0E(m, obj);
        return 0;
    }
    if (ld8(m, obj + O_STATE) == 5u) {
        follow_parent(m, obj, a1, 1, -8);
        sub_0D0690(m, obj);
        return 0;
    }
    if (S32(ld32(m, obj + O_HP)) <= 0) {
        uint32_t child;

        sub_0D0E04(m, obj, 0x12Cu);
        child = Obj_Spawn(m, obj, 0x00DC328u, 3u, 2u, 0u);
        if (S32(child) >= 0) {
            wr32(m, child + O_X, ld32(m, obj + O_X));
            wr16(m, child + O_X, U16(ld16(m, child + O_X) + 0x20u));
            wr32(m, child + O_Y, ld32(m, obj + O_Y));
            wr16(m, child + O_Y, U16(ld16(m, child + O_Y) + 0x20u));
            wr16(m, child + 0x58u, 0xB4u);
            wr16(m, child + 0x60u, 4u);
        }
        a1 = ld32(m, obj + O_CURSOR);
        wr8(m, a1 + O_STATE, 4u);
        follow_parent(m, obj, a1, 1, -8);
        sub_0D0678(m, obj, 0x0DF536u);
        wr8(m, obj + O_COLOR, 0u);
        wr8(m, obj + O_STATE, 5u);
        return 0;
    }
    follow_parent(m, obj, a1, -0x10, -0x21);
    switch (ld8(m, obj + O_STATE)) {
    case 1: rc = sub_0DEE10(m, obj, a1); break;
    case 2: rc = sub_0DEED8(m, obj, a1); break;
    case 3: rc = sub_0DEF62(m, obj, a1); break;
    case 4: rc = sub_0DEFC0(m, obj, a1); break;
    default: return -1; /* unimplemented: ROM 0x0DEDF8 */
    }
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    return rc;
}
