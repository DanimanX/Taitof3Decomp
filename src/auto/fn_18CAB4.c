/* ROM 0x18CAB4: boss main controller tick (A0 = object).
 * Copies HP (+0x24) to A5+0x4B0C. If state A5+0x4B38 == -1, spawns 0x17AE0A
 * death handler and transitions to 0x18CAF0. Spawns 0x17AD18 on A5+0x4B26
 * bit 5, 0x17ABDC on bomb-active flag, dispatches A5+0x4B38 through +0x64
 * (0x17AE7C, 0x18CFB6, 0x18D0AC, 0x18D58C, 0x18D6AE), clamps +0x50 to
 * [-0x64, 0x19A] and +0x54 to [-0x32, 0x12C], handles A5+0x4B3A bit 14
 * phase-transition spawns (0x18CC64 / 0x18CD2A), updates hit-flash +0x1D /
 * A5+0x4B36, runs 0x18CC14 threshold check, and re-arms 0x18CAB4. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static inline uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static inline int16_t abs16(int16_t v)
{
    return v < 0 ? (int16_t)(0u - (uint16_t)v) : v;
}

/* ROM 0x17B02E: resolve score receiver from +0x35 (+0x36->+8 if 1, +0x36 if 2, else -1). */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        *a1_io = a1;
        return rd32(m, a1 + 0x08u);
    }
    if (mode == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x18CC14: update phase threshold A5+0x4B26 bit 4 (0x10) vs A5+0x4B10. */
static void sub_18CC14(const f3mem_t *m, uint32_t *d0_io)
{
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x10u));
    int32_t d0 = (int32_t)rd32(m, F3_A5 + 0x4B10u);
    if (d0 > (int32_t)rd32(m, F3_A5 + 0x4B0Cu)) {
        d0 = (int32_t)rd32(m, F3_A5 + 0x4B18u);
        wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - (uint32_t)d0);
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x10u));
    }
    *d0_io = (uint32_t)d0;
}

/* ROM 0x179074 with D0/D1/A1 register tracking. */
static void scr_179074_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    int16_t d2 = (int16_t)(int8_t)rd8(m, obj + 0x42u);
    if (d2 < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    int16_t ex = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t ey = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    *d0_io = setw(*d0_io, (uint16_t)ex);
    *d1_io = setw(*d1_io, (uint16_t)ey);
    if (!(abs16(ex) > d2 || abs16(ey) > d2)) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    uint16_t ang = Obj_Atan2(m, (uint16_t)ex, (uint16_t)ey);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t jitter = rd16(m, a1 + 8u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    uint8_t b = (uint8_t)(((uint8_t)ang & (uint8_t)jitter) + (uint8_t)(int8_t)(int16_t)(jitter >> 8));
    wr8(m, obj + 0x20u, b);

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    *a1_io = 0x00027E40u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    *d0_io = (uint32_t)vx;
    *d1_io = (uint32_t)vy;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178C06 with D0/D1/A1 register tracking. */
static void scr_178C06_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    uint16_t d2w = (uint16_t)(rd16(m, obj + 0x20u) + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, d0w);
    wr16(m, obj + 0x20u, d2w);
    int16_t mag = (int16_t)(uint16_t)(d0w + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(d2w >> 8);
    *a1_io = 0x00027E40u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    *d0_io = (uint32_t)vx;
    *d1_io = (uint32_t)vy;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178FCE: steer toward (+0x4C, +0x4E) within |+0x42.b| threshold. */
static void scr_178FCE_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    int16_t d2 = (int16_t)(int8_t)rd8(m, obj + 0x42u);
    if (d2 < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    int16_t ex = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t ey = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    *d0_io = setw(*d0_io, (uint16_t)ex);
    *d1_io = setw(*d1_io, (uint16_t)ey);
    if (!(abs16(ex) > d2 || abs16(ey) > d2)) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    uint16_t ang = Obj_Atan2(m, (uint16_t)ex, (uint16_t)ey);
    uint32_t a1 = rd32(m, obj + 0x44u);
    *a1_io = a1;
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t spd = rd16(m, a1 + 8u);
    int8_t diff = (int8_t)(uint8_t)((uint8_t)ang - rd8(m, obj + 0x20u));
    if (diff == 0)
        spd = 0u;
    else if (diff < 0)
        spd = (uint16_t)(0u - spd);

    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + spd));

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    *a1_io = 0x00027E40u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    *d0_io = (uint32_t)vx;
    *d1_io = (uint32_t)vy;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x18DA60: set (+0x4C, +0x4E) to (arg0.w, clamp(player_y + 0x10, 0x3C, 0xAB)). */
static void scr_18DA60(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d0w = rd16(m, a1 + 6u);
    int16_t d1w = 0x0078;
    int32_t p = (int32_t)rd32(m, F3_A5 + 0x1D28u);
    if (p > 0) {
        a1 = (uint32_t)p;
        d1w = (int16_t)(rd16(m, a1 + 0x18u) + 0x0010u);
        if (d1w < 0x003C)
            d1w = 0x003C;
        if (d1w > 0x00AB)
            d1w = 0x00AB;
    }
    wr16(m, obj + 0x4Cu, d0w);
    wr16(m, obj + 0x4Eu, (uint16_t)d1w);
    *d0_io = setw(*d0_io, d0w);
    *d1_io = setw(*d1_io, (uint16_t)d1w);
    *a1_io = a1;
}

/* ROM 0x178982 with local commands + D0/D1/A1 register tracking. */
static int script10_step_18CAB4(const f3mem_t *m, uint32_t obj,
                                uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a1;

    for (;;) {
        a1 = rd32(m, obj + 0x44u);
        *a1_io = a1;
        uint32_t fn = rd32(m, a1);

        if (fn == 0u)
            goto loop_marker;

        if (fn == 0x00179074u) {
            scr_179074_regs(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x00178C06u) {
            scr_178C06_regs(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x00178FCEu) {
            scr_178FCE_regs(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x0018DA60u) {
            scr_18DA60(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x00178AC6u) {
            wr16(m, obj + 0x50u, rd16(m, a1 + 6u));
            wr16(m, obj + 0x54u, rd16(m, a1 + 8u));
            wr16(m, obj + 0x52u, 0u);
            wr16(m, obj + 0x56u, 0u);
        } else if (fn == 0x00178B0Eu) {
            wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
            wr16(m, obj + 0x20u, rd16(m, a1 + 8u));
        } else if (fn == 0x00178B20u) {
            Obj_Script10_SetHeading(m, obj);
        } else if (fn == 0x00178B7Au) {
            int16_t x0 = (int16_t)rd16(m, a1 + 6u);
            int16_t y0 = (int16_t)rd16(m, a1 + 8u);
            wr16(m, obj + 0x4Cu, (uint16_t)x0);
            wr16(m, obj + 0x4Eu, (uint16_t)y0);
            int32_t dx = (int16_t)(uint16_t)(x0 - (int16_t)rd16(m, obj + 0x50u));
            int32_t dy = (int16_t)(uint16_t)(y0 - (int16_t)rd16(m, obj + 0x54u));
            int16_t n = (int16_t)rd16(m, obj + 0x48u);
            if (n > 0) {
                wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - (uint16_t)(dx / n)));
                wr16(m, obj + 0x4Eu, (uint16_t)(rd16(m, obj + 0x4Eu) - (uint16_t)(dy / n)));
            }
            wr16(m, obj + 0x48u, 0u);
        } else if (fn == 0x0017921Au) {
            *a1_io = fn; /* bare rts */
        } else if (fn == 0x00179222u) {
            int16_t v0 = (int16_t)rd16(m, a1 + 6u);
            if (v0 >= 0) {
                uint16_t mask = (uint16_t)(0x8000u >> ((uint16_t)v0 & 15u));
                wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) | mask);
            }
            int16_t v1 = (int16_t)rd16(m, a1 + 8u);
            if (v1 >= 0) {
                uint16_t mask = (uint16_t)(0x8000u >> ((uint16_t)v1 & 15u));
                wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) | mask);
            }
            *d0_io = (uint32_t)(int32_t)v1;
        } else if (fn == 0x0017924Cu) {
            int16_t v0 = (int16_t)rd16(m, a1 + 6u);
            if (v0 >= 0) {
                uint16_t mask = (uint16_t)(0x8000u >> ((uint16_t)v0 & 15u));
                wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) & (uint16_t)~mask);
            }
            int16_t v1 = (int16_t)rd16(m, a1 + 8u);
            if (v1 >= 0) {
                uint16_t mask = (uint16_t)(0x8000u >> ((uint16_t)v1 & 15u));
                wr16(m, F3_A5 + 0x4B3Au, rd16(m, F3_A5 + 0x4B3Au) & (uint16_t)~mask);
            }
            *d0_io = (uint32_t)(int32_t)v1;
        } else if (fn == 0x00179276u) {
            int16_t c0 = (int16_t)rd16(m, a1 + 6u);
            if (c0 >= 0)
                Sound_PlayCmd(m, Sound_PreCmdHook(m, c0));
            int16_t c1 = (int16_t)rd16(m, a1 + 8u);
            if (c1 >= 0)
                Sound_PlayCmd(m, Sound_PreCmdHook(m, c1));
            *d0_io = (uint32_t)(int32_t)c1;
        } else if (fn == 0x001792B0u) {
            int16_t c0 = (int16_t)rd16(m, a1 + 6u);
            if (c0 >= 0)
                Sound_PlayCmdForced(m, Sound_PreCmdHook(m, c0));
            int16_t c1 = (int16_t)rd16(m, a1 + 8u);
            if (c1 >= 0)
                Sound_PlayCmdForced(m, Sound_PreCmdHook(m, c1));
            *d0_io = (uint32_t)(int32_t)c1;
        } else if (fn == 0x001792EAu) {
            *d0_io = Obj_Spawn(m, obj, 0x0017933Au, 0u, 1u, rd32(m, a1 + 6u));
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x04u));
        } else {
            return Obj_Script10(m, obj);
        }

        int16_t n = (int16_t)rd16(m, obj + 0x48u);
        if (n < 0)
            return 0;
        if (n > 0) {
            n = (int16_t)(n - 1);
            wr16(m, obj + 0x48u, (uint16_t)n);
            if (n > 0)
                return 0;
            a1 = rd32(m, obj + 0x44u) + 0x0Au;
            if ((int32_t)rd32(m, a1) > 0) {
                *a1_io = a1;
                wr32(m, obj + 0x44u, a1);
                wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                return 0;
            }
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        *a1_io = a1;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }

loop_marker: {
        int16_t c = (int16_t)rd16(m, obj + 0x4Au);
        if (c >= 0) {
            if (c == 0)
                wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
            c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
            wr16(m, obj + 0x4Au, (uint16_t)c);
            if (c == 0) {
                a1 += 0x0Au;
                goto store;
            }
        }
        a1 = rd32(m, a1 + 6u);
    }

store:
    *a1_io = a1;
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    return 0;
}

/* ROM 0x18D0AC / 0x18D6AE: shared phase-script state runner. */
static int run_phase_script(const f3mem_t *m, uint32_t obj, uint16_t limit,
                            uint32_t idx_tbl, uint32_t ptr_tbl, uint16_t cur_state,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
        if ((int16_t)rd16(m, obj + 0x62u) >= (int16_t)limit)
            wr16(m, obj + 0x62u, 0u);
        uint16_t seq = rd16(m, obj + 0x62u);
        uint16_t idx = rd16(m, idx_tbl + (uint32_t)(int32_t)(int16_t)seq * 2u);
        uint32_t a1 = rd32(m, ptr_tbl + (uint32_t)(int32_t)(int16_t)idx * 4u);
        *a1_io = a1;
        *d0_io = setw(*d0_io, idx);
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        wr16(m, obj + 0x4Au, 0u);
        wr16(m, obj + 0x62u, (uint16_t)(seq + 1u));
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x04u));
    }
    if (script10_step_18CAB4(m, obj, d0_io, d1_io, a1_io) < 0)
        return -1;
    if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
        wr16(m, F3_A5 + 0x4B38u, cur_state);
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
    }
    return 0;
}

int Fn_18CAB4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    wr32(m, F3_A5 + 0x4B0Cu, rd32(m, obj + 0x24u));           /* 0x18CAB4 */
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {            /* 0x18CABA */
        d0 = Obj_Spawn(m, obj, 0x0017AE0Au, 0u, 1u, 0u);      /* 0x18CAC4..0x18CADC */
        wr16(m, obj + 0x06u, 1u);                             /* 0x18CAE0: beq 18CAF6 never taken */
        wr32(m, obj, 0x0018CAF0u);                            /* 0x18CAE8 */
        r->d[0] = d0;
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x4B26u) & 0x20u) != 0u) {            /* 0x18CB04 */
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x20u));
        d0 = Obj_Spawn(m, obj, 0x0017AD18u, 0u, 1u, 0x0017AD9Cu); /* 0x18CB14 -> 0x18CBF4 */
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&            /* 0x18CB18 */
        (rd8(m, obj + 0x23u) & 0x80u) == 0u) {                /* 0x18CB22 */
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x80u));
        d0 = Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x00000080u); /* 0x18CB32..0x18CB4C */
    }

    int16_t state = (int16_t)rd16(m, F3_A5 + 0x4B38u);        /* 0x18CB50 */
    d0 = setw(d0, (uint16_t)state);
    uint32_t tbl = rd32(m, obj + 0x64u);                      /* 0x18CB54 */
    a1 = rd32(m, tbl + (uint32_t)((int32_t)state * 4));       /* 0x18CB58 */

    if (a1 == 0x0017AE7Cu) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            a1 = 0x0017AF48u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            d1 = ((uint32_t)rd8(m, obj + 0x17u) << 24)
               | ((uint32_t)rd8(m, obj + 0x1Bu) << 16)
               | (uint32_t)rd16(m, F3_A5 + 0x4B30u);
            d0 = Obj_Spawn(m, obj, 0x0017A5A8u, 5u, 2u, d1);
            wr32(m, obj + 0x0Cu, d0);
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x01u));
            wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x10u));
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x04u));
            d1 = (uint32_t)rd16(m, F3_A5 + 0x4B32u);
            d0 = Obj_Spawn(m, obj, 0x0013E9E8u, 0u, 1u, d1);
        }
        if (script10_step_18CAB4(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        uint16_t sx = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t sy = (uint16_t)(rd16(m, obj + 0x54u) + rd16(m, obj + 0x1Au));
        d0 = setw(d0, sx);
        d1 = setw(d1, sy);
        wr16(m, F3_A5 + 0x32FAu, sx);
        wr16(m, F3_A5 + 0x32FCu, sy);
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 0xFFFFu);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
        }
    } else if (a1 == 0x0018CFB6u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            a1 = 0x0018D008u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x01u));
        }
        if (script10_step_18CAB4(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 2u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x01u));
        }
    } else if (a1 == 0x0018D0ACu) {
        if (run_phase_script(m, obj, 0x000Au, 0x0018D1A4u, 0x0018D170u, 2u, &d0, &d1, &a1) < 0)
            return -1;
        if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x1C20)
            wr32(m, obj + 0x24u, 0u);
        if ((int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {
            wr16(m, F3_A5 + 0x4B38u, 3u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            d0 = sub_17B02E(m, obj, &a1);
            if ((int32_t)d0 >= 0) {
                a1 = d0;
                wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u);
            }
            wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);
            wr16(m, F3_A5 + 0x4B3Au, (uint16_t)(rd16(m, F3_A5 + 0x4B3Au) & 0x001Fu)); /* {0:11} */
            wr16(m, obj + 0x62u, 0u);
        }
    } else if (a1 == 0x0018D58Cu) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x08u));
            a1 = 0x0018D5F6u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) | 0x01u));
            wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x04u));
            wr8(m, F3_A5 + 0x4B3Au, (uint8_t)(rd8(m, F3_A5 + 0x4B3Au) & ~0x01u));
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x04u));
        }
        if (script10_step_18CAB4(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 4u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x01u));
            wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) & ~0x04u));
        }
    } else if (a1 == 0x0018D6AEu) {
        if (run_phase_script(m, obj, 7u, 0x0018D796u, 0x0018D768u, 4u, &d0, &d1, &a1) < 0)
            return -1;
        if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x3840)
            wr32(m, obj + 0x24u, 0u);
        if ((int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {
            wr16(m, F3_A5 + 0x4B38u, 0u);
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x08u));
            d0 = sub_17B02E(m, obj, &a1);
            if ((int32_t)d0 >= 0) {
                a1 = d0;
                wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x000186A0u);
            }
            wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);
        }
    } else {
        return -1;
    }

    if ((int16_t)rd16(m, obj + 0x50u) < -0x0064)                /* 0x18CB5E */
        wr16(m, obj + 0x50u, 0xFF9Cu);
    if ((int16_t)rd16(m, obj + 0x50u) > 0x019A)                 /* 0x18CB6E */
        wr16(m, obj + 0x50u, 0x019Au);
    if ((int16_t)rd16(m, obj + 0x54u) < -0x0032)                /* 0x18CB7E */
        wr16(m, obj + 0x54u, 0xFFCEu);
    if ((int16_t)rd16(m, obj + 0x54u) > 0x012C)                 /* 0x18CB8E */
        wr16(m, obj + 0x54u, 0x012Cu);

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u) {            /* 0x18CB9E {14:1} */
        wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x02u)); /* 0x18CC46 */
        if ((int16_t)rd16(m, F3_A5 + 0x4B7Eu) != 1) {         /* 0x18CC4C */
            uint16_t ph = (uint16_t)(rd16(m, F3_A5 + 0x4B7Eu) + 1u); /* 0x18CC56 */
            wr16(m, F3_A5 + 0x4B7Eu, ph);
            d0 = setw(d0, ph);
            a1 = rd32(m, 0x0018CC3Eu + (uint32_t)(int32_t)(int16_t)ph * 4u); /* 0x18CC5E */
            if (a1 == 0x0018CC64u) {
                Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0018E1AAu);
                Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0018E1BAu);
                Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0018E1CAu);
                Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0018E1DAu);
                Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0018E1EAu);
                d0 = Obj_Spawn(m, obj, 0x0017AAEEu, 3u, 2u, 0x0018E1FAu);
                wr32(m, obj + 0x24u, 0x01F40000u);
                Obj_18495E(m, obj);
                d0 = setw(d0, 0u);
                d1 = setw(d1, 0u);
            } else if (a1 == 0x0018CD2Au) {
                a1 = 0x0018EA50u;
                if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x08u) != 0u)
                    a1 = 0x00156004u;
                d0 = Obj_Spawn(m, obj, a1, 0u, 1u, 0u);
                wr32(m, obj + 0x0Cu, d0);
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x50u) - 0x0059u));
                wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x54u) - 0x0040u));
                wr16(m, obj + 0x06u, 2u);
                wr32(m, obj, 0x0018CD84u);
                r->d[0] = d0;
                r->d[1] = d1;
                r->a[1] = a1;
                return 0;
            } else {
                return -1;
            }
        }
    }

    wr8(m, F3_A5 + 0x4B36u, 0u);                              /* 0x18CBAC */
    uint8_t c28 = rd8(m, F3_A5 + 0x4B28u);                    /* 0x18CBB0 */
    if (c28 == rd8(m, obj + 0x1Du)) {                         /* 0x18CBB4 */
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B29u));         /* 0x18CBD8 */
    } else if (rd8(m, obj + 0x35u) == 0u) {                   /* 0x18CBBC */
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));         /* 0x18CBC4 */
    } else {
        wr8(m, obj + 0x1Du, c28);                             /* 0x18CBCE */
    }
    wr8(m, obj + 0x35u, 0u);                                  /* 0x18CBE2 */
    wr8(m, F3_A5 + 0x4B36u, rd8(m, obj + 0x1Du));             /* 0x18CBE6 */

    sub_18CC14(m, &d0);                                       /* 0x18CBEC */
    wr32(m, obj, 0x0018CAB4u);                                /* 0x18CBF0 -> 0x18CAAC */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
