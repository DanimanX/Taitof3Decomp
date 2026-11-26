/* ROM 0x195BAA: boss main controller tick (A0 = object).
 * Caps HP (+0x24) to 0 when stage timer A5+0x3318 >= 0x3840, copies HP to
 * A5+0x4B0C. If state A5+0x4B38 == -1, spawns 0x17AE0A death handler and
 * transitions to 0x195BF8 (note: move.w #1,+6 makes beq 0x195BFE never taken).
 * If A5+0x4B38 > 0 and HP <= 0, resets state to 0, clears A5+0x4B26 bit 4
 * and A5+0x4B3A phase flags, and awards 0x186A0 via 0x17B02E. Copies A5+0x4B3A
 * to A5+0x4BA0, spawns 0x17ABDC on bomb-active flag, dispatches A5+0x4B38
 * through the +0x64 table (0x17AE7C, 0x195D70, 0x195E64, 0x195F18, 0x195FCC),
 * updates hit-flash colour +0x1D / A5+0x4B36, runs 0x17A642 phase HP threshold
 * check, ramps A5+0x4BA2..A5+0x4BA6, spawns 0x181EA0 when A5+0x4B3A MSB is
 * set, and re-arms 0x195BAA. */
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

/* ROM 0x195E3A: set A5+0x4B9E bit 7 (MSB) when arg0.w >= 0 or arg1.w >= 0. */
static void scr_195E3A(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    *a1_io = a1;
    int32_t d0 = (int32_t)(int16_t)rd16(m, a1 + 6u);
    if (d0 >= 0)
        wr8(m, F3_A5 + 0x4B9Eu, rd8(m, F3_A5 + 0x4B9Eu) | 0x80u);
    d0 = (int32_t)(int16_t)rd16(m, a1 + 8u);
    if (d0 >= 0)
        wr8(m, F3_A5 + 0x4B9Eu, rd8(m, F3_A5 + 0x4B9Eu) | 0x80u);
    *d0_io = (uint32_t)d0;
}

/* ROM 0x1967EE: track player Y-10 with 0x179074, clamp +0x54 to [0, 0xC8]. */
static void scr_1967EE(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    int32_t p = (int32_t)rd32(m, F3_A5 + 0x1D28u);
    *d0_io = (uint32_t)p;
    if (p <= 0)
        return;
    uint32_t a1 = (uint32_t)p;
    *a1_io = a1;
    uint16_t ty = (uint16_t)(rd16(m, a1 + 0x18u) - 10u);
    *d0_io = setw(*d0_io, ty);
    wr16(m, obj + 0x4Eu, ty);
    scr_179074_regs(m, obj, d0_io, d1_io, a1_io);
    if ((int16_t)rd16(m, obj + 0x54u) < 0)
        wr16(m, obj + 0x54u, 0u);
    if ((int16_t)rd16(m, obj + 0x54u) > 0x00C8)
        wr16(m, obj + 0x54u, 0x00C8u);
}

/* ROM 0x19682C: track player Y-2 up/down (0xC0/0x40) via 0x178C06, clamp [0, 0xC8]. */
static void scr_19682C(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    int32_t p = (int32_t)rd32(m, F3_A5 + 0x1D28u);
    *d0_io = (uint32_t)p;
    if (p <= 0)
        return;
    uint32_t a1 = (uint32_t)p;
    *a1_io = a1;
    int16_t py = (int16_t)(rd16(m, a1 + 0x18u) - 2u);
    *d0_io = setw(*d0_io, (uint16_t)py);
    wr8(m, obj + 0x20u, (py > (int16_t)rd16(m, obj + 0x54u)) ? 0x40u : 0xC0u);
    scr_178C06_regs(m, obj, d0_io, d1_io, a1_io);
    if ((int16_t)rd16(m, obj + 0x54u) < 0)
        wr16(m, obj + 0x54u, 0u);
    if ((int16_t)rd16(m, obj + 0x54u) > 0x00C8)
        wr16(m, obj + 0x54u, 0x00C8u);
}

/* ROM 0x178982 with local commands + D0/D1/A1 register tracking. */
static int script10_step_195BAA(const f3mem_t *m, uint32_t obj,
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
        } else if (fn == 0x00195E3Au) {
            scr_195E3A(m, obj, d0_io, a1_io);
        } else if (fn == 0x001967EEu) {
            scr_1967EE(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x0019682Cu) {
            scr_19682C(m, obj, d0_io, d1_io, a1_io);
        } else {
            if (fn == 0x00178AC6u || fn == 0x00178B0Eu || fn == 0x00178B20u ||
                fn == 0x00178B7Au || fn == 0x0017921Au || fn == 0x00179222u ||
                fn == 0x0017924Cu || fn == 0x00179276u || fn == 0x00179312u) {
                /* Execute single command without advancing via Obj_Script10? */
                /* Wait: Obj_Script10 advances the script too; only run our single-step loop
                 * by handling the rest inline so register tracking and loop state stay exact. */
                if (fn == 0x00178AC6u) {
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
                } else if (fn == 0x00179312u) {
                    *d0_io = Obj_Spawn(m, obj, 0x0017933Au, 3u, 2u, rd32(m, a1 + 6u));
                    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) & ~0x04u));
                }
            } else {
                return Obj_Script10(m, obj);
            }
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

/* ROM 0x195E64 / 0x195F18 / 0x195FCC: shared phase-script state runner. */
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
        uint32_t a1 = rd32(m, 0x00196050u + (uint32_t)(int32_t)(int16_t)idx * 4u);
        *a1_io = a1;
        *d0_io = setw(*d0_io, idx);
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        wr16(m, obj + 0x4Au, 0u);
        wr16(m, obj + 0x62u, (uint16_t)(seq + 1u));
        wr8(m, F3_A5 + 0x4B3Au, rd8(m, F3_A5 + 0x4B3Au) & ~0x70u); /* {2:2} and {1:1} */
        wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) & ~0x0Cu); /* {12:2} */
        wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x04u);  /* {5:1} */
    }
    if (script10_step_195BAA(m, obj, d0_io, d1_io, a1_io) < 0)
        return -1;
    if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
        if (has_hp_advance &&
            (int32_t)rd32(m, F3_A5 + 0x4B10u) > (int32_t)rd32(m, F3_A5 + 0x4B0Cu)) {
            wr16(m, F3_A5 + 0x4B38u, next_state);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            uint32_t step = rd32(m, F3_A5 + 0x4B18u) * 3u;
            *d0_io = step;
            wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - step);
            wr16(m, obj + 0x62u, 0u);
        } else {
            wr16(m, F3_A5 + 0x4B38u, cur_state);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        }
    }
    return 0;
}

int Fn_195BAA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x3840)    /* 0x195BAA */
        wr32(m, obj + 0x24u, 0u);

    wr32(m, F3_A5 + 0x4B0Cu, rd32(m, obj + 0x24u));     /* 0x195BBC */
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {      /* 0x195BC2 */
        d0 = Obj_Spawn(m, obj, 0x0017AE0Au, 0u, 1u, 0u);
        wr16(m, obj + 0x06u, 1u);                       /* 0x195BE8: beq 195BFE never taken */
        wr32(m, obj, 0x00195BF8u);                      /* 0x195BF0 */
        r->d[0] = d0;
        return 0;
    }

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) > 0 &&        /* 0x195C0C */
        (int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {
        wr16(m, F3_A5 + 0x4B38u, 0u);                   /* 0x195C1C */
        wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        d0 = sub_17B02E(m, obj, &a1);                   /* 0x195C28 */
        if ((int32_t)d0 >= 0) {
            a1 = d0;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x000186A0u);
        }
        wr8(m, F3_A5 + 0x4B3Au, rd8(m, F3_A5 + 0x4B3Au) & ~0x02u); /* {6:1} */
        wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) & ~0x1Cu); /* {11,12,13:1} */
    }

    wr16(m, F3_A5 + 0x4BA0u, rd16(m, F3_A5 + 0x4B3Au)); /* 0x195C56 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&      /* 0x195C5C */
        (rd8(m, obj + 0x23u) & 0x80u) == 0u) {
        wr8(m, obj + 0x23u, rd8(m, obj + 0x23u) | 0x80u);
        d0 = Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x00000080u);
    }

    /* 0x195C94..0x195CA0: dispatch via (+0x64)[A5+0x4B38] */
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
        if (script10_step_195BAA(m, obj, &d0, &d1, &a1) < 0)
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
    } else if (a1 == 0x00195D70u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
            a1 = 0x00195DC2u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) | 0x01u);
        }
        if (script10_step_195BAA(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 2u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) & ~0x01u);
        }
    } else if (a1 == 0x00195E64u) {
        if (run_phase_script(m, obj, 5u, 0x0019607Cu, 3u, 2u, 1, &d0, &d1, &a1) < 0)
            return -1;
    } else if (a1 == 0x00195F18u) {
        if (run_phase_script(m, obj, 6u, 0x00196086u, 4u, 3u, 1, &d0, &d1, &a1) < 0)
            return -1;
    } else if (a1 == 0x00195FCCu) {
        if (run_phase_script(m, obj, 7u, 0x00196092u, 4u, 4u, 0, &d0, &d1, &a1) < 0)
            return -1;
    } else {
        return -1;
    }

    /* 0x195CA2..0x195CE2 */
    wr8(m, F3_A5 + 0x4B36u, 0u);
    if (rd8(m, F3_A5 + 0x4B28u) == rd8(m, obj + 0x1Du))
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B29u));
    else if (rd8(m, obj + 0x35u) == 0u)
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B28u));
    wr8(m, obj + 0x35u, 0u);
    wr8(m, F3_A5 + 0x4B36u, rd8(m, obj + 0x1Du));

    sub_17A642(m, &d0);                                 /* 0x195CE2 */

    /* 0x195CE8..0x195D2A */
    wr16(m, F3_A5 + 0x4BA2u, 0x000Au);
    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x40u) != 0u) {
        wr16(m, F3_A5 + 0x4BA6u, 0x0600u);
        if (rd16(m, F3_A5 + 0x4BA4u) != 0u)
            wr16(m, F3_A5 + 0x4BA4u, (uint16_t)(rd16(m, F3_A5 + 0x4BA4u) - 1u));
    } else {
        wr16(m, F3_A5 + 0x4BA6u, 0x0400u);
        if (rd16(m, F3_A5 + 0x4BA4u) != 0x0060u)
            wr16(m, F3_A5 + 0x4BA4u, (uint16_t)(rd16(m, F3_A5 + 0x4BA4u) + 1u));
    }

    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x80u) != 0u)        /* 0x195D2A */
        d0 = Obj_Spawn(m, obj, 0x00181EA0u, 9u, 2u, 0x40100000u);

    wr8(m, F3_A5 + 0x4B3Au, rd8(m, F3_A5 + 0x4B3Au) & ~0x80u); /* 0x195D52 */
    wr32(m, obj, 0x00195BAAu);                          /* 0x195D58 -> 0x195BA2 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
