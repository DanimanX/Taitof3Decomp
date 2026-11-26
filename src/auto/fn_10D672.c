/* ROM 0x10D672: boss/enemy articulated arm/pod sub-object tick (A0 = object,
 * A0+0x08 = parent). If parent+0x06 == 0, clears +0x06; for old state > 1
 * spawns 0x0DC008 explosions along the 0x0D1EBC(+0x68, 0x10) vector, plays
 * sound 0x34, and starts dying (0x0CEE74). If +0x24 <= 0, awards 0x3E8 via
 * 0x0D0E04, sets parent+0x22 bit (+0x42), clears +0x06, spawns 0x0DC008
 * explosions, and plays sound 0x34. Otherwise sets (+0x3A, +0x3E) = parent
 * (+0x14, +0x18) + (0x10, 0x10), dispatches state (+0x06) via jump table
 * 0x10D800:
 *   1: when parent+0x06 == 4, plays sound 0x11 and advances +0x06 = 2.
 *   2: spawns 0x10DA5E smoke every 4 ticks, steps 0x0D0690/0x0D0DC2/0x0CBE34,
 *      adds +5 to +0x44, moves (+0x3A, +0x3E) by 0x0D25E4(+0x68, +0x44),
 *      switches to state 3 with sound 0x2B on terrain hit or +0x3E outside
 *      [-8, 0xF0].
 *   3: spawns 0x10DA5E smoke every 4 ticks, steps 0x0D0690/0x0D0DC2/0x0CBE34,
 *      moves (+0x3A, +0x3E) by 0x0D25E4(+0x68, +0x44), and clears +0x06 / sets
 *      parent+0x22 bit (+0x42) when offscreen left.
 * Finally copies (+0x3A - 8, +0x3E - 8) into (+0x14, +0x18). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

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

/* ROM 0x0D25E4: multi-step vector scale (0x0D1EDC/0x0D2044 + 0x0D263C/0x0D27A4). */
static void sub_0D25E4(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d4w = 0u;
    while (d1w >= 0x0032u) {
        d4w = (uint16_t)(d4w + 1u);
        d1w = (uint16_t)(d1w - 0x0032u);
    }
    int32_t idx = (int32_t)(int16_t)d0w;
    uint32_t d0 = rd32(m, (uint32_t)(0x000D1EDCu + idx * 4)) * (uint32_t)d1w;
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2044u + idx * 4)) * (uint32_t)d1w;
    uint32_t hi0 = rd32(m, (uint32_t)(0x000D263Cu + idx * 4)) * (uint32_t)d4w;
    uint32_t hi1 = rd32(m, (uint32_t)(0x000D27A4u + idx * 4)) * (uint32_t)d4w;
    *out_d0 = d0 + hi0;
    *out_d1 = d1 + hi1;
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

/* ROM 0x10D6F0..0x10D75E / 0x10D790..0x10D7FE: spawn 0x0DC008 explosions along
 * (+0x58, +0x60) until within 0x0C of parent+0x14 + 8, then play sound 0x34. */
static void spawn_arm_explosions(const f3mem_t *m, uint32_t obj, uint32_t a4,
                                 uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t v0, v1;
    sub_0D1EBC(m, rd16(m, obj + 0x68u), 0x0010u, &v0, &v1);
    wr32(m, obj + 0x58u, v0);
    wr32(m, obj + 0x60u, v1);
    *d0_io = v0;
    *d1_io = v1;

    for (;;) {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC008u, 3u, 2u, 0u);
        *d0_io = child;
        if ((int32_t)child < 0)
            break;
        *a1_io = child;
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        uint32_t sx = rd32(m, obj + 0x58u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - sx);
        uint32_t sy = rd32(m, obj + 0x60u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - sy);
        int16_t diff = (int16_t)(rd16(m, a4 + 0x14u) + 0x10 - rd16(m, obj + 0x14u) - 8);
        if (diff < 0)
            diff = (int16_t)(-diff);
        *d0_io = setw(sy, (uint16_t)diff);
        if ((uint16_t)diff <= 0x000Cu)
            break;
    }

    Sound_PlayHooked(m, 0x0034u);
}

int Fn_10D672(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a4 = rd32(m, obj + 0x08u);                 /* 0x10D672 */
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, a4 + 0x06u) == 0u) {                     /* 0x10D676 */
        uint8_t prev = rd8(m, obj + 0x06u);             /* 0x10D6CA */
        d0 = (d0 & 0xFFFFFF00u) | prev;
        wr8(m, obj + 0x06u, 0u);
        if (prev <= 1u) {                               /* 0x10D6D2 */
            Obj_StartDying(m, obj);
            r->d[0] = d0;
            r->a[4] = a4;
            return 0;
        }
        spawn_arm_explosions(m, obj, a4, &d0, &d1, &a1);
        Obj_StartDying(m, obj);                         /* 0x10D75E */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        r->a[4] = a4;
        return 0;
    }

    if (rd8(m, obj + 0x06u) == 0u) {                    /* 0x10D67E */
        r->a[4] = a4;
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x10D686 */
        d0 = 0x000003E8u;                               /* 0x10D764 */
        uint8_t mode = rd8(m, obj + 0x35u);
        if (mode == 1u)
            a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
        else if (mode == 2u)
            a1 = rd32(m, obj + 0x36u);
        Obj_0D0E04(m, obj, d0);                         /* 0x10D76A */
        uint16_t bit = rd16(m, obj + 0x42u);            /* 0x10D770 */
        wr8(m, a4 + 0x22u, rd8(m, a4 + 0x22u) | (uint8_t)(1u << (bit & 7u)));
        wr8(m, obj + 0x06u, 0u);                        /* 0x10D778 */
        spawn_arm_explosions(m, obj, a4, &d0, &d1, &a1);
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        r->a[4] = a4;
        return 0;
    }

    /* 0x10D68E..0x10D6A0: anchor (+0x3A, +0x3E) at parent (+0x14, +0x18) + 0x10 */
    wr32(m, obj + 0x3Au, rd32(m, a4 + 0x14u));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 0x0010u));
    wr32(m, obj + 0x3Eu, rd32(m, a4 + 0x18u));
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 0x0010u));

    uint8_t state = rd8(m, obj + 0x06u);
    d0 = setw(d0, state);                               /* 0x10D6A6..0x10D6A8 */

    switch (state) {
    case 1u:                                            /* 0x10D810 */
        if (rd8(m, a4 + 0x06u) == 4u) {
            Sound_PlayHooked(m, 0x0011u);
            wr8(m, obj + 0x06u, 2u);
        }
        break;
    case 2u:                                            /* 0x10D836 */
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t == 0u) {
                wr8(m, obj + 0x23u, 4u);
                d0 = Obj_Spawn(m, obj, 0x0010DA5Eu, 9u, 2u, 0u);
            }
            step_script_regs(m, obj, &d0, &a1);         /* 0x10D860 */
            add_coll_regs(m, obj, &a1);                 /* 0x10D866 */
            Obj_BlinkColour(m, obj);                    /* 0x10D86C */
            uint16_t len = (uint16_t)(rd16(m, obj + 0x44u) + 5u);
            wr16(m, obj + 0x44u, len);                  /* 0x10D872 */
            sub_0D25E4(m, rd16(m, obj + 0x68u), len, &d0, &d1); /* 0x10D87E */
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
            wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);
            int16_t tx = (int16_t)rd16(m, obj + 0x3Au);
            int16_t ty = (int16_t)rd16(m, obj + 0x3Eu);
            d0 = setw(d0, (uint16_t)tx);
            d1 = setw(d1, (uint16_t)ty);
            terrain_test_regs(m, tx, ty, &d0, &d1);     /* 0x10D894 */
            if ((uint8_t)d0 != 0u || ty < -8 || ty > 0x00F0) {
                Sound_PlayHooked(m, 0x002Bu);           /* 0x10D8A0 */
                wr8(m, obj + 0x06u, 3u);
            }
        }
        break;
    case 3u:                                            /* 0x10D8CE */
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t == 0u) {
                wr8(m, obj + 0x23u, 4u);
                d0 = Obj_Spawn(m, obj, 0x0010DA5Eu, 9u, 2u, 0u);
            }
            step_script_regs(m, obj, &d0, &a1);         /* 0x10D8F8 */
            add_coll_regs(m, obj, &a1);                 /* 0x10D8FE */
            Obj_BlinkColour(m, obj);                    /* 0x10D904 */
            sub_0D25E4(m, rd16(m, obj + 0x68u), rd16(m, obj + 0x44u), &d0, &d1);
            wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
            wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);
            uint16_t arm_id = rd16(m, obj + 0x42u);     /* 0x10D920 */
            d0 = setw(d0, arm_id);
            int offscreen;
            if (arm_id == 0u || arm_id == 3u)
                offscreen = ((int16_t)rd16(m, obj + 0x14u) < -0x10);
            else
                offscreen = ((int16_t)rd16(m, a4 + 0x14u) < -0x20);
            if (offscreen) {
                d0 = setw(d0, rd16(m, obj + 0x42u));    /* 0x10D948 */
                wr8(m, a4 + 0x22u, rd8(m, a4 + 0x22u) | (uint8_t)(1u << (d0 & 7u)));
                wr8(m, obj + 0x06u, 0u);
            }
        }
        break;
    default:
        return -1;
    }

    /* 0x10D6B4..0x10D6C4 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
