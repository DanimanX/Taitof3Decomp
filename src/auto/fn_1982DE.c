/* ROM 0x1982DE: boss #2 main controller tick (A0 = object).
 * Caps HP (+0x24) to 0 when stage timer A5+0x3318 >= 0x3840, copies HP to
 * A5+0x4B0C. If state A5+0x4B38 == -1, spawns 0x17AE0A death handler and
 * transitions to 0x19832C (note: move.w #1,+6 makes beq 0x198332 never taken).
 * If A5+0x4B38 > 0 and HP <= 0 and state != 5, transitions to state 5, clears
 * A5+0x4B26 bit 4, and awards 0x186A0 via 0x17B02E. Spawns 0x17AD18 on
 * A5+0x4B26 bit 2, spawns 0x17ABDC on bomb-active flag, dispatches A5+0x4B38
 * through the +0x64 table (0x17AE7C, 0x199074, 0x1991F0, 0x19928C, 0x199328,
 * 0x199AEE), clamps (+0x50, +0x54) to [-0x64..0x17C] x [-0x78..0x156], updates
 * hit-flash colour +0x1D / A5+0x4B36, runs 0x17A642 phase HP threshold check,
 * spawns 0x1984C0 when A5+0x4BA8 bits 3..4 are clear and bit 5 wasn't set,
 * and re-arms 0x1982DE. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static int16_t abs16(int16_t v)
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

/* ROM 0x17A642: update phase threshold A5+0x4B26 bit 4 (0x10) vs A5+0x4B14. */
static void sub_17A642(const f3mem_t *m, uint32_t *d0_io)
{
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x10u));
    uint32_t d0 = rd32(m, F3_A5 + 0x4B14u);
    if ((int32_t)d0 > (int32_t)rd32(m, F3_A5 + 0x4B0Cu)) {
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x10u));
        d0 = rd32(m, F3_A5 + 0x4B18u);
        wr32(m, F3_A5 + 0x4B14u, rd32(m, F3_A5 + 0x4B14u) - d0);
    }
    *d0_io = d0;
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

/* ROM 0x178FCE with D0/D1/A1 register tracking. */
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

/* ROM 0x199A20: accelerate (+0x42, +0x5C) toward player and integrate pos. */
static void scr_199A20(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    int32_t p = (int32_t)rd32(m, F3_A5 + 0x1D28u);
    *d0_io = (uint32_t)p;
    if (p > 0) {
        uint32_t a2 = (uint32_t)p;
        uint32_t a1 = rd32(m, obj + 0x44u);
        *a1_io = a1;
        int16_t d0w = (int16_t)rd16(m, a1 + 6u);
        if (d0w != 0) {
            int16_t tx = (int16_t)(rd16(m, a2 + 0x14u) + 0x18);
            int16_t x = (int16_t)rd16(m, obj + 0x50u);
            if (tx != x) {
                if (tx <= x)
                    d0w = (int16_t)(-d0w);
                int16_t vx = (int16_t)(rd16(m, obj + 0x42u) + d0w);
                if (vx < -0x0200)
                    vx = -0x0200;
                if (vx > 0x0200)
                    vx = 0x0200;
                wr16(m, obj + 0x42u, (uint16_t)vx);
            }
        }
        d0w = (int16_t)rd16(m, a1 + 8u);
        if (d0w != 0) {
            int16_t ty = (int16_t)(rd16(m, a2 + 0x18u) + 0x10);
            int16_t y = (int16_t)rd16(m, obj + 0x54u);
            if (ty != y) {
                if (ty <= y)
                    d0w = (int16_t)(-d0w);
                int16_t vy = (int16_t)(rd16(m, obj + 0x5Cu) + d0w);
                if (vy < -0x0200)
                    vy = -0x0200;
                if (vy > 0x0200)
                    vy = 0x0200;
                wr16(m, obj + 0x5Cu, (uint16_t)vy);
            }
        }
    }

    int32_t vx = (int32_t)(int16_t)(rd16(m, obj + 0x42u) + 0x20u) << 8;
    int32_t vy = (int32_t)(int16_t)(rd16(m, obj + 0x5Cu) + 0x20u) << 8;
    vx = (int32_t)(((uint32_t)vx & 0xFFFF0000u) | ((uint32_t)vx & 0xC000u));
    vy = (int32_t)(((uint32_t)vy & 0xFFFF0000u) | ((uint32_t)vy & 0xC000u));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);

    uint16_t ang = Obj_Atan2(m, rd16(m, obj + 0x42u), rd16(m, obj + 0x5Cu));
    wr8(m, obj + 0x20u, (uint8_t)ang);
    *d0_io = setw((uint32_t)vx, ang);
    *d1_io = (uint32_t)vy;
}

/* ROM 0x178982 with 1982DE-specific commands and D0/D1/A1 tracking. */
static int script10_step_1982DE(const f3mem_t *m, uint32_t obj,
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
        } else if (fn == 0x00199A20u) {
            scr_199A20(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x001995DEu) {
            scr_199A20(m, obj, d0_io, d1_io, a1_io);
            int16_t y = (int16_t)rd16(m, obj + 0x54u);
            int16_t vy = (int16_t)rd16(m, obj + 0x5Cu);
            if (y <= 0x0046 && vy <= 0) {
                wr8(m, F3_A5 + 0x4B3Au, rd8(m, F3_A5 + 0x4B3Au) & ~0x04u);
                wr16(m, obj + 0x48u, 0x0010u);
            } else if (y >= 0x00B0 && vy >= 0) {
                wr8(m, F3_A5 + 0x4B3Au, rd8(m, F3_A5 + 0x4B3Au) & ~0x04u);
                wr16(m, obj + 0x48u, 0x0010u);
            }
        } else if (fn == 0x0019961Eu) {
            if ((int16_t)rd16(m, obj + 0x54u) < 0)
                wr16(m, obj + 0x54u, 0u);
            if ((int16_t)rd16(m, obj + 0x54u) > 0x00E8)
                wr16(m, obj + 0x54u, 0x00E8u);
            wr16(m, obj + 0x4Eu, 0x0074u);
            wr16(m, obj + 0x4Cu, rd16(m, obj + 0x50u));
        } else if (fn == 0x00199190u) {
            *d0_io = Obj_Spawn(m, obj, 0x0016711Eu, 0u, 1u, 0u);
        } else if (fn == 0x001991B0u) {
            *d0_io = Obj_Spawn(m, obj, 0x00167192u, 0u, 1u, 0u);
        } else if (fn == 0x001996CEu) {
            uint16_t v = rd16(m, F3_A5 - 0x1456u) & 3u;
            *d0_io = setw(*d0_io, v);
            wr16(m, F3_A5 + 0x4BB2u, v);
        } else if (fn == 0x001996DCu) {
            if ((rd8(m, F3_A5 + 0x4BA8u) & 0x18u) == 0u)
                wr16(m, obj + 0x48u, 0u);
        } else if (fn == 0x00199944u) {
            uint16_t tx = 0x005Au, ty = 0x003Cu;
            if ((rd8(m, F3_A5 + 0x4BA9u) & 0x40u) != 0u) {
                tx = 0x00A0u;
                ty = 0x0070u;
            }
            *d0_io = setw(*d0_io, tx);
            *d1_io = setw(*d1_io, ty);
            wr16(m, obj + 0x4Cu, tx);
            wr16(m, obj + 0x4Eu, ty);
        } else if (fn == 0x00199968u) {
            uint16_t tx = 0x00DCu, ty = 0x003Cu;
            if ((rd8(m, F3_A5 + 0x4BA9u) & 0x20u) != 0u) {
                tx = 0x00B4u;
                ty = 0x0070u;
            }
            *d0_io = setw(*d0_io, tx);
            *d1_io = setw(*d1_io, ty);
            wr16(m, obj + 0x4Cu, tx);
            wr16(m, obj + 0x4Eu, ty);
        } else if (fn == 0x00199A0Eu) {
            wr16(m, obj + 0x4Cu, rd16(m, a1 + 6u));
            wr16(m, obj + 0x4Eu, rd16(m, obj + 0x54u));
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
        } else if (fn == 0x00178B38u) {
            wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
            wr16(m, obj + 0x5Cu, rd16(m, a1 + 8u));
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
            /* rts */
        } else if (fn == 0x0017921Cu) {
            wr16(m, obj + 0x4Au, 0u);
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

/* ROM 0x1991F0 / 0x19928C / 0x199328: shared phase-script state runner. */
static int run_phase_script(const f3mem_t *m, uint32_t obj, uint16_t limit,
                            uint32_t idx_tbl, uint16_t next_state, uint16_t cur_state,
                            int has_hp_advance,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
        wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
        if ((int16_t)rd16(m, obj + 0x62u) >= (int16_t)limit)
            wr16(m, obj + 0x62u, 0u);
        uint16_t seq = rd16(m, obj + 0x62u);
        uint16_t idx = rd16(m, idx_tbl + (uint32_t)(int32_t)(int16_t)seq * 2u);
        uint32_t a1 = rd32(m, 0x001993A8u + (uint32_t)(int32_t)(int16_t)idx * 4u);
        *a1_io = a1;
        *d0_io = setw(*d0_io, idx);
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        wr16(m, obj + 0x4Au, 0u);
        wr16(m, obj + 0x62u, (uint16_t)(seq + 1u));
    }
    if (script10_step_1982DE(m, obj, d0_io, d1_io, a1_io) < 0)
        return -1;
    if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
        if (has_hp_advance) {
            if ((int32_t)rd32(m, F3_A5 + 0x4B10u) <= (int32_t)rd32(m, F3_A5 + 0x4B0Cu)) {
                wr16(m, F3_A5 + 0x4B38u, cur_state);
                wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            } else {
                uint32_t step = rd32(m, F3_A5 + 0x4B18u) * 3u;
                *d0_io = step;
                wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - step);
                wr16(m, F3_A5 + 0x4B38u, next_state);
                wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            }
        } else {
            wr16(m, F3_A5 + 0x4B38u, next_state);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            if ((rd8(m, F3_A5 + 0x4BA8u) & 0x20u) != 0u) /* {2:1} */
                wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) | 0x20u); /* {10:1} */
        }
    }
    return 0;
}

int Fn_1982DE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x3840)    /* 0x1982DE */
        wr32(m, obj + 0x24u, 0u);

    wr32(m, F3_A5 + 0x4B0Cu, rd32(m, obj + 0x24u));     /* 0x1982F0 */
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {      /* 0x1982F6 */
        d0 = Obj_Spawn(m, obj, 0x0017AE0Au, 0u, 1u, 0u);
        wr16(m, obj + 0x06u, 1u);                       /* 0x19831C: beq 198332 never taken */
        wr32(m, obj, 0x0019832Cu);                      /* 0x198324 */
        r->d[0] = d0;
        return 0;
    }

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) > 0 &&        /* 0x198340 */
        (int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0 &&
        (int16_t)rd16(m, F3_A5 + 0x4B38u) != 5) {
        wr16(m, F3_A5 + 0x4B38u, 5u);
        wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        d0 = sub_17B02E(m, obj, &a1);
        if ((int32_t)d0 >= 0) {
            a1 = d0;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x000186A0u);
        }
    }

    if ((rd8(m, F3_A5 + 0x4B26u) & 0x20u) != 0u) {      /* 0x19837C {2:1} */
        wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x20u);
        d0 = Obj_Spawn(m, obj, 0x0017AD18u, 0u, 1u, 0x0017AD9Cu);
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&      /* 0x198390 */
        (rd8(m, obj + 0x23u) & 0x80u) == 0u) {
        wr8(m, obj + 0x23u, rd8(m, obj + 0x23u) | 0x80u);
        d0 = Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x00000040u);
    }

    /* 0x1983C8..0x1983D4: dispatch via (+0x64)[A5+0x4B38] */
    int16_t state = (int16_t)rd16(m, F3_A5 + 0x4B38u);
    d0 = setw(d0, (uint16_t)state);
    uint32_t tbl = rd32(m, obj + 0x64u);
    a1 = rd32(m, tbl + (uint32_t)((int32_t)state * 4));

    if (a1 == 0x0017AE7Cu) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
            a1 = 0x0017AF48u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            d1 = ((uint32_t)rd8(m, obj + 0x17u) << 24) |
                 ((uint32_t)rd8(m, obj + 0x1Bu) << 16) |
                 (uint32_t)rd16(m, F3_A5 + 0x4B30u);
            d0 = Obj_Spawn(m, obj, 0x0017A5A8u, 5u, 2u, d1);
            wr32(m, obj + 0x0Cu, d0);
            wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) | 0x01u);
            wr8(m, F3_A5 - 0x144Cu, rd8(m, F3_A5 - 0x144Cu) | 0x10u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x04u);
            d1 = (uint32_t)rd16(m, F3_A5 + 0x4B32u);
            d0 = Obj_Spawn(m, obj, 0x0013E9E8u, 0u, 1u, d1);
        }
        if (script10_step_1982DE(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        uint16_t sx = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t sy = (uint16_t)(rd16(m, obj + 0x54u) + rd16(m, obj + 0x1Au));
        d0 = setw(d0, sx);
        d1 = setw(d1, sy);
        wr16(m, F3_A5 + 0x32FAu, sx);
        wr16(m, F3_A5 + 0x32FCu, sy);
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 0xFFFFu);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            Obj_Free(m, rd32(m, obj + 0x0Cu));
        }
    } else if (a1 == 0x00199074u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
            a1 = 0x001990DCu;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) | 0x01u);
            d0 = Obj_Spawn(m, obj, 0x00198F6Eu, 3u, 2u, 0u);
        }
        if (script10_step_1982DE(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 2u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        }
    } else if (a1 == 0x001991F0u) {
        if (run_phase_script(m, obj, 8u, 0x001993C8u, 3u, 2u, 1, &d0, &d1, &a1) < 0)
            return -1;
    } else if (a1 == 0x0019928Cu) {
        if (run_phase_script(m, obj, 8u, 0x001993D8u, 4u, 3u, 1, &d0, &d1, &a1) < 0)
            return -1;
    } else if (a1 == 0x00199328u) {
        if (run_phase_script(m, obj, 6u, 0x001993E8u, 4u, 4u, 0, &d0, &d1, &a1) < 0)
            return -1;
    } else if (a1 == 0x00199AEEu) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
            a1 = 0x00199B82u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            wr16(m, F3_A5 + 0x4B3Au,
                 (uint16_t)((rd16(m, F3_A5 + 0x4B3Au) | 0x0101u) & ~0x06FEu));
        }
        if (script10_step_1982DE(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 0u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            wr16(m, obj + 0x16u, 0x0018u);
            wr16(m, obj + 0x1Au, 0x0008u);
        }
    } else {
        return -1;
    }

    /* 0x1983D6..0x198416: clamp (+0x50, +0x54) */
    if ((int16_t)rd16(m, obj + 0x50u) < -0x0064)
        wr16(m, obj + 0x50u, (uint16_t)-0x0064);
    if ((int16_t)rd16(m, obj + 0x50u) > 0x017C)
        wr16(m, obj + 0x50u, 0x017Cu);
    if ((int16_t)rd16(m, obj + 0x54u) < -0x0078)
        wr16(m, obj + 0x54u, (uint16_t)-0x0078);
    if ((int16_t)rd16(m, obj + 0x54u) > 0x0156)
        wr16(m, obj + 0x54u, 0x0156u);

    /* 0x198416..0x19845C */
    wr8(m, F3_A5 + 0x4B36u, 0u);
    if (rd8(m, F3_A5 + 0x4B28u) == rd8(m, obj + 0x1Du))
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B29u));
    else if (rd8(m, obj + 0x35u) == 0u)
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B28u));
    wr8(m, obj + 0x35u, 0u);
    wr8(m, F3_A5 + 0x4B36u, rd8(m, obj + 0x1Du));

    sub_17A642(m, &d0);                                 /* 0x198456 */

    /* 0x19845C..0x19849C */
    if ((rd8(m, F3_A5 + 0x4BA8u) & 0x18u) != 0u) {
        wr8(m, F3_A5 + 0x4BA8u, rd8(m, F3_A5 + 0x4BA8u) & ~0x04u);
    } else if ((rd8(m, F3_A5 + 0x4BA8u) & 0x04u) == 0u) {
        wr8(m, F3_A5 + 0x4BA8u, rd8(m, F3_A5 + 0x4BA8u) | 0x04u);
        d0 = Obj_Spawn(m, obj, 0x001984C0u, 5u, 2u, 0u);
    }

    wr32(m, obj, 0x001982DEu);                          /* 0x19849C -> 0x1982D6 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
