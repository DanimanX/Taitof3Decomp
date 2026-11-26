/* ROM 0x19C648: boss main state machine tick (A0 = object).
 * Forces HP = 0 when A5+0x3318 >= 0x3840, copies HP to A5+0x4B0C.
 * If A5+0x4B38 == -1, spawns 0x17AE0A (group 0, count 1), sets +0x06 = 1,
 * and arms 0x19C696. If A5+0x4B38 > 0 and A5+0x4B0C <= 0, resets state to 0,
 * clears A5+0x4B26 bit 4 (0x08), awards 0x186A0 via 0x17B02E, and clears
 * A5+0x4B3A/4B3B phase flags. Copies A5+0x4B3A to A5+0x4BC0, spawns
 * 0x17ABDC on bomb flag (A5+0x1D58 bit 1), dispatches via (+0x64)[A5+0x4B38]
 * (0x17AE7C, 0x19C7C4, 0x19C9E4, 0x19C890, 0x19CA84, 0x19C93A, 0x19CB2A),
 * updates hit-flash colour +0x1D / A5+0x4B36, runs 0x17A642 phase HP threshold
 * check, and re-arms 0x19C648. */
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
    return (v < 0) ? (int16_t)(0u - (uint16_t)v) : v;
}

/* ROM 0x17B02E: resolve score receiver from +0x35 (+0x36->+8 if 1, +0x36 if 2, else -1). */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        *a1_io = a1;
        return rd32(m, a1 + 8u);
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

/* ROM 0x178B68: set +0x42 and +0x60 from record args. */
static void scr_178B68_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    *a1_io = a1;
    wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
    wr16(m, obj + 0x60u, rd16(m, a1 + 8u));
}

/* ROM 0x178BBC: step +0x42/+0x60/+0x20 and advance (+0x50, +0x54) along 0x27E40. */
static void scr_178BBC_regs(const f3mem_t *m, uint32_t obj,
                            uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint16_t d0w = rd16(m, obj + 0x42u);
    uint16_t d2w = rd16(m, obj + 0x60u);
    uint32_t a1 = rd32(m, obj + 0x44u);
    d0w = (uint16_t)(d0w + rd16(m, a1 + 6u));
    d2w = (uint16_t)(d2w + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, d0w);
    wr16(m, obj + 0x60u, d2w);
    d2w = (uint16_t)(d2w + rd16(m, obj + 0x20u));
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

/* ROM 0x178982 with D0/D1/A1 register tracking. */
static int script10_step_19C648(const f3mem_t *m, uint32_t obj,
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
        } else if (fn == 0x00178B68u) {
            scr_178B68_regs(m, obj, a1_io);
        } else if (fn == 0x00178BBCu) {
            scr_178BBC_regs(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x00178C06u) {
            scr_178C06_regs(m, obj, d0_io, d1_io, a1_io);
        } else if (fn == 0x00178FCEu) {
            scr_178FCE_regs(m, obj, d0_io, d1_io, a1_io);
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
            /* rts no-op */
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

/* Helper to start a script from 0x19CB9A via an index table (0x19CBDA/EE/F6). */
static void start_indexed_script(const f3mem_t *m, uint32_t obj, uint16_t limit,
                                 uint32_t idx_tbl, uint32_t *d0_io, uint32_t *a1_io)
{
    wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
    if ((int16_t)rd16(m, obj + 0x62u) >= (int16_t)limit)
        wr16(m, obj + 0x62u, 0u);
    uint16_t seq = rd16(m, obj + 0x62u);
    uint16_t idx = rd16(m, idx_tbl + (uint32_t)(int32_t)(int16_t)seq * 2u);
    uint32_t a1 = rd32(m, 0x0019CB9Au + (uint32_t)(int32_t)(int16_t)idx * 4u);
    *d0_io = setw(*d0_io, idx);
    *a1_io = a1;
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    wr16(m, obj + 0x4Au, 0u);
    wr16(m, obj + 0x62u, (uint16_t)(seq + 1u));
}

int Fn_19C648(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int16_t)rd16(m, F3_A5 + 0x3318u) >= 0x3840)      /* 0x19C648 */
        wr32(m, obj + 0x24u, 0u);                         /* 0x19C652 */

    wr32(m, F3_A5 + 0x4B0Cu, rd32(m, obj + 0x24u));       /* 0x19C65A */

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {        /* 0x19C660 */
        d0 = Obj_Spawn(m, obj, 0x0017AE0Au, 0u, 1u, 0u);  /* 0x19C66A..0x19C67C */
        wr16(m, obj + 0x06u, 1u);                         /* 0x19C686 (Z=0, beq 19C69C never taken) */
        wr32(m, obj, 0x0019C696u);                        /* 0x19C68E */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) > 0 &&          /* 0x19C6AA */
        (int32_t)rd32(m, F3_A5 + 0x4B0Cu) <= 0) {         /* 0x19C6B2 */
        wr16(m, F3_A5 + 0x4B38u, 0u);                     /* 0x19C6BA */
        wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u); /* 0x19C6C0 */
        d0 = sub_17B02E(m, obj, &a1);                     /* 0x19C6C6 */
        if ((int32_t)d0 >= 0) {                           /* 0x19C6CC */
            a1 = d0;                                      /* 0x19C6D2 */
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x000186A0u); /* 0x19C6D4 */
        }
        wr8(m, F3_A5 + 0x4B3Au, rd8(m, F3_A5 + 0x4B3Au) & (uint8_t)~0xFAu); /* 0x19C6DC..0x19C6FA */
        wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) & (uint8_t)~0x98u); /* 0x19C700..0x19C70C */
    }

    wr16(m, F3_A5 + 0x4BC0u, rd16(m, F3_A5 + 0x4B3Au));   /* 0x19C712 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&        /* 0x19C718 */
        (rd8(m, obj + 0x23u) & 0x80u) == 0u) {            /* 0x19C722 */
        wr8(m, obj + 0x23u, rd8(m, obj + 0x23u) | 0x80u); /* 0x19C72C */
        d0 = Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x00000080u); /* 0x19C732..0x19C746 */
    }

    /* 0x19C750..0x19C75C: dispatch via (+0x64)[A5+0x4B38] */
    int16_t state = (int16_t)rd16(m, F3_A5 + 0x4B38u);
    d0 = setw(d0, (uint16_t)state);
    a1 = rd32(m, rd32(m, obj + 0x64u) + (uint32_t)((int32_t)state * 4));

    if (a1 == 0x0017AE7Cu) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {    /* 0x17AE7C */
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
        if (script10_step_19C648(m, obj, &d0, &d1, &a1) < 0)
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
    } else if (a1 == 0x0019C7C4u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {    /* 0x19C7C4 */
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
            a1 = 0x0019C82Cu;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
            wr8(m, F3_A5 + 0x4B3Bu, rd8(m, F3_A5 + 0x4B3Bu) | 0x01u); /* {15:1} */
            d0 = Obj_Spawn(m, obj, 0x0019EFAAu, 5u, 2u, 0u);
        }
        if (script10_step_19C648(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 2u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        }
    } else if (a1 == 0x0019C9E4u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u)     /* 0x19C9E4 */
            start_indexed_script(m, obj, 0x000Au, 0x0019CBDAu, &d0, &a1);
        if (script10_step_19C648(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            d0 = rd32(m, F3_A5 + 0x4B10u);
            if ((int32_t)d0 <= (int32_t)rd32(m, F3_A5 + 0x4B0Cu)) {
                wr16(m, F3_A5 + 0x4B38u, 2u);
                wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            } else {
                d0 = rd32(m, F3_A5 + 0x4B18u) * 2u;
                wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - d0);
                wr16(m, F3_A5 + 0x4B38u, 3u);
                wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
                wr16(m, obj + 0x62u, 0u);
            }
        }
    } else if (a1 == 0x0019C890u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {    /* 0x19C890 */
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
            a1 = 0x0019C8D6u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
        }
        if (script10_step_19C648(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 4u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        }
    } else if (a1 == 0x0019CA84u) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u)     /* 0x19CA84 */
            start_indexed_script(m, obj, 0x0004u, 0x0019CBEEu, &d0, &a1);
        if (script10_step_19C648(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            if ((int16_t)rd16(m, F3_A5 + 0x4BC2u) < 0x0013 &&
                (int32_t)(d0 = rd32(m, F3_A5 + 0x4B10u)) <= (int32_t)rd32(m, F3_A5 + 0x4B0Cu)) {
                wr16(m, F3_A5 + 0x4B38u, 4u);
                wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            } else {
                d0 = rd32(m, F3_A5 + 0x4B18u) * 3u;
                wr32(m, F3_A5 + 0x4B10u, rd32(m, F3_A5 + 0x4B10u) - d0);
                wr16(m, F3_A5 + 0x4B38u, 5u);
                wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
            }
        }
    } else if (a1 == 0x0019C93Au) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u) {    /* 0x19C93A */
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x08u);
            a1 = 0x0019C980u;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
        }
        if (script10_step_19C648(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 6u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        }
    } else if (a1 == 0x0019CB2Au) {
        if ((rd8(m, F3_A5 + 0x4B26u) & 0x08u) == 0u)     /* 0x19CB2A */
            start_indexed_script(m, obj, 0x0008u, 0x0019CBF6u, &d0, &a1);
        if (script10_step_19C648(m, obj, &d0, &d1, &a1) < 0)
            return -1;
        if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
            wr16(m, F3_A5 + 0x4B38u, 6u);
            wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) & ~0x08u);
        }
    } else {
        return -1;
    }

    /* 0x19C75E..0x19C79E */
    wr8(m, F3_A5 + 0x4B36u, 0u);
    if (rd8(m, F3_A5 + 0x4B28u) == rd8(m, obj + 0x1Du))
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B29u));
    else if (rd8(m, obj + 0x35u) == 0u)
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B28u));
    wr8(m, obj + 0x35u, 0u);
    wr8(m, F3_A5 + 0x4B36u, rd8(m, obj + 0x1Du));

    sub_17A642(m, &d0);                                   /* 0x19C79E */
    wr32(m, obj, 0x0019C648u);                            /* 0x19C7A4 -> 0x19C640 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
