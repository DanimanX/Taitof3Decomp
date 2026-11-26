/* ROM 0x133720: boss multi-part joint/limb pose controller tick (A0 = object).
 * Dispatches init (f0, when A5+0x30EE bit 12 is set and cleared) and tick (f1)
 * routines from the 9-entry table 0x133762 indexed by A5+0x30F2 (0..8), driving
 * joint angles A5+0x32B0..0x32CA and boss Y/scroll via 0x1337AA/C0/CE/DE and
 * 0x13459A/0x134622. Re-arms 0x133720 while parent (+0x08)->+0x59 == 0 else
 * frees via 0x0105EC. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

typedef struct {
    uint16_t off;
    uint16_t target;
} pose_target_t;

typedef struct {
    uint16_t off;
    uint16_t target;
    uint16_t step;
} pose_step_t;

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x1337AA: signed slewing step of (a1).w toward d0w clamped to +-d1w. */
static uint16_t step_1337AA(const f3mem_t *m, f3regs_t *r,
                            uint16_t d0w, uint16_t d1w, uint32_t a1)
{
    int16_t diff = (int16_t)(d0w - rd16(m, a1));
    int16_t lim = (int16_t)d1w;
    if (diff > lim) {
        diff = lim;
    } else {
        lim = (int16_t)(-lim);
        if (diff < lim)
            diff = lim;
    }
    wr16(m, a1, (uint16_t)(rd16(m, a1) + (uint16_t)diff));
    r->d[0] = setw(r->d[0], (uint16_t)diff);
    r->d[1] = setw(r->d[1], (uint16_t)lim);
    r->a[1] = a1;
    return (uint16_t)diff;
}

/* ROM 0x1337C0: unsigned upward step of (a1).w toward d0w clamped to d1w. */
static void step_1337C0(const f3mem_t *m, f3regs_t *r,
                        uint16_t d0w, uint16_t d1w, uint32_t a1)
{
    uint16_t diff = (uint16_t)(d0w - rd16(m, a1));
    if (diff > d1w)
        diff = d1w;
    wr16(m, a1, (uint16_t)(rd16(m, a1) + diff));
    r->d[0] = setw(r->d[0], diff);
    r->d[1] = setw(r->d[1], d1w);
    r->a[1] = a1;
}

/* ROM 0x1337CE: unsigned downward step of (a1).w toward d0w clamped to d1w. */
static void step_1337CE(const f3mem_t *m, f3regs_t *r,
                        uint16_t d0w, uint16_t d1w, uint32_t a1)
{
    uint16_t diff = (uint16_t)(rd16(m, a1) - d0w);
    if (diff > d1w)
        diff = d1w;
    wr16(m, a1, (uint16_t)(rd16(m, a1) - diff));
    r->d[0] = setw(r->d[0], diff);
    r->d[1] = setw(r->d[1], d1w);
    r->a[1] = a1;
}

/* ROM 0x1337DE: proportional step (d0w - (a1).w) / d1w into (a1).w. */
static void step_1337DE(const f3mem_t *m, f3regs_t *r,
                        uint16_t d0w, uint16_t d1w, uint32_t a1)
{
    int32_t num = (int16_t)(d0w - rd16(m, a1));
    int16_t den = (int16_t)d1w;
    int16_t q = (int16_t)(num / den);
    int16_t rem = (int16_t)(num % den);
    wr16(m, a1, (uint16_t)(rd16(m, a1) + (uint16_t)q));
    r->d[0] = ((uint32_t)(uint16_t)rem << 16) | (uint16_t)q;
    r->d[1] = setw(r->d[1], d1w);
    r->a[1] = a1;
}

static void run_targets(const f3mem_t *m, f3regs_t *r,
                        const pose_target_t *tbl, uint32_t n, uint16_t step)
{
    for (uint32_t i = 0; i < n; i++)
        (void)step_1337AA(m, r, tbl[i].target, step, F3_A5 + tbl[i].off);
}

static uint16_t run_steps_or(const f3mem_t *m, f3regs_t *r,
                             const pose_step_t *tbl, uint32_t n)
{
    uint16_t d7 = 0u;
    for (uint32_t i = 0; i < n; i++)
        d7 |= step_1337AA(m, r, tbl[i].target, tbl[i].step, F3_A5 + tbl[i].off);
    return d7;
}

/* ROM 0x13457C: initialize 4 joint cycle slots at obj+0x44..0x52. */
static void sub_13457C(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x44u, 0u);
    wr16(m, obj + 0x46u, 0u);
    wr16(m, obj + 0x48u, 0x000Cu);
    wr16(m, obj + 0x4Au, 0u);
    wr16(m, obj + 0x4Cu, 0x0006u);
    wr16(m, obj + 0x4Eu, 0u);
    wr16(m, obj + 0x50u, 0u);
    wr16(m, obj + 0x52u, 0u);
}

/* ROM 0x134622: advance one joint cycle slot and read 0x13466C target pair. */
static void sub_134622(const f3mem_t *m, uint32_t a2, uint8_t d0b,
                       uint16_t *d1_out, uint16_t *d2_out, uint16_t *d3_out)
{
    uint16_t d4w = rd16(m, a2);
    uint32_t rec = 0x0013466Cu + (uint32_t)(int32_t)(int16_t)d4w;
    *d1_out = rd16(m, rec + 2u);
    *d2_out = rd16(m, rec + 4u);
    int16_t d3 = (int16_t)rd16(m, rec);

    uint16_t cnt = (uint16_t)(rd16(m, a2 + 2u) + 1u);
    wr16(m, a2 + 2u, cnt);
    d3 = (int16_t)(d3 - (int16_t)cnt);
    if (d3 <= 0) {
        wr16(m, a2 + 2u, 0u);
        uint8_t d4b = (rd8(m, F3_A5 + 0x30EFu) & 0x10u) ? 0xFFu : 0x00u;
        if ((d4b ^ d0b) == 0u) {
            int16_t idx = (int16_t)(rd16(m, a2) + 6u);
            wr16(m, a2, (idx > 0x0C) ? 0u : (uint16_t)idx);
        } else {
            int16_t idx = (int16_t)(rd16(m, a2) - 6u);
            wr16(m, a2, (idx < 0) ? 0x000Cu : (uint16_t)idx);
        }
    }
    *d3_out = (uint16_t)(d3 + 1);
}

/* ROM 0x13459A: drive 4 arm/leg joint pairs from obj+0x44..0x52. */
static void sub_13459A(const f3mem_t *m, f3regs_t *r)
{
    static const struct { uint16_t off0, off1; uint8_t d0b; } slots[4] = {
        { 0x32C0u, 0x32C2u, 0x00u },
        { 0x32C8u, 0x32CAu, 0x00u },
        { 0x32BCu, 0x32BEu, 0xFFu },
        { 0x32C4u, 0x32C6u, 0xFFu },
    };
    uint32_t obj = r->a[0];
    for (int i = 0; i < 4; i++) {
        uint16_t d1, d2, d3;
        sub_134622(m, obj + 0x44u + (uint32_t)i * 4u, slots[i].d0b, &d1, &d2, &d3);
        step_1337DE(m, r, d1, d3, F3_A5 + slots[i].off0);
        step_1337DE(m, r, d2, d3, F3_A5 + slots[i].off1);
    }
}

/* State 0 tick: ROM 0x1337EA. */
static void tick_state0(const f3mem_t *m, f3regs_t *r)
{
    static const pose_target_t tbl[14] = {
        { 0x32B6u, 0x8800u }, { 0x32B0u, 0x8800u },
        { 0x32B8u, 0x7000u }, { 0x32B2u, 0x7000u },
        { 0x32BAu, 0x1800u }, { 0x32B4u, 0x1800u },
        { 0x32C0u, 0x8800u }, { 0x32BCu, 0x8800u },
        { 0x32C2u, 0x5000u }, { 0x32BEu, 0x5000u },
        { 0x32C8u, 0x9000u }, { 0x32C4u, 0x9000u },
        { 0x32CAu, 0x5800u }, { 0x32C6u, 0x5800u },
    };
    run_targets(m, r, tbl, 14u, 0x0190u);
}

/* State 1 tick: ROM 0x1338CA. */
static void tick_state1(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t sub = (int16_t)rd16(m, obj + 0x22u);
    int32_t fn = (int32_t)rd32(m, 0x001338EEu + (int32_t)sub * 4);
    if (fn <= 0) {
        wr8(m, F3_A5 + 0x30F2u, 0u);
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u));
        r->d[0] = (uint32_t)fn;
        return;
    }

    static const pose_step_t sub0[3] = {
        { 0x32B6u, 0xC000u, 0x0258u }, { 0x32B8u, 0xC000u, 0x0258u },
        { 0x32BAu, 0x0000u, 0x0320u },
    };
    static const pose_step_t sub1[3] = {
        { 0x32B0u, 0xC000u, 0x0258u }, { 0x32B2u, 0xC000u, 0x0258u },
        { 0x32B4u, 0x0000u, 0x0320u },
    };
    static const pose_step_t sub2[2] = {
        { 0x32C0u, 0xA000u, 0x0190u }, { 0x32C2u, 0x8000u, 0x0190u },
    };
    static const pose_step_t sub3[2] = {
        { 0x32BCu, 0xA000u, 0x0190u }, { 0x32BEu, 0x8000u, 0x0190u },
    };
    static const pose_step_t sub4[2] = {
        { 0x32C8u, 0x9800u, 0x0190u }, { 0x32CAu, 0x8000u, 0x0190u },
    };
    static const pose_step_t sub5[2] = {
        { 0x32C4u, 0x9800u, 0x0190u }, { 0x32C6u, 0x8000u, 0x0190u },
    };
    static const pose_target_t sub6a[8] = {
        { 0x32C0u, 0x8800u }, { 0x32C2u, 0x6800u },
        { 0x32BCu, 0x8800u }, { 0x32BEu, 0x6800u },
        { 0x32C8u, 0x8000u }, { 0x32CAu, 0x6000u },
        { 0x32C4u, 0x8000u }, { 0x32C6u, 0x6000u },
    };
    static const pose_step_t sub6b[4] = {
        { 0x32B6u, 0x7800u, 0x0258u }, { 0x32B8u, 0x7800u, 0x0258u },
        { 0x32B0u, 0x7800u, 0x0258u }, { 0x32B2u, 0x7800u, 0x0258u },
    };
    static const pose_target_t sub7[14] = {
        { 0x32C0u, 0x7000u }, { 0x32C2u, 0x4800u },
        { 0x32BCu, 0x7000u }, { 0x32BEu, 0x4800u },
        { 0x32C8u, 0x6000u }, { 0x32CAu, 0x3800u },
        { 0x32C4u, 0x6000u }, { 0x32C6u, 0x3800u },
        { 0x32B6u, 0x4000u }, { 0x32B8u, 0x4000u },
        { 0x32BAu, 0x4000u }, { 0x32B0u, 0x4000u },
        { 0x32B2u, 0x4000u }, { 0x32B4u, 0x4000u },
    };

    switch (sub) {
    case 0:
        if (run_steps_or(m, r, sub0, 3u) == 0u) {
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
            wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x20u));
            Sound_PlayHooked(m, 0x0040u);
        }
        break;
    case 1:
        if (run_steps_or(m, r, sub1, 3u) == 0u) {
            wr8(m, F3_A5 + 0x32CCu, 0xDEu);
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
            wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x20u));
            Sound_PlayHooked(m, 0x0040u);
        }
        break;
    case 2: case 3: case 4: case 5: {
        const pose_step_t *p = (sub == 2) ? sub2 : (sub == 3) ? sub3 : (sub == 4) ? sub4 : sub5;
        if (run_steps_or(m, r, p, 2u) == 0u) {
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
            wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x20u));
            Sound_PlayHooked(m, 0x001Bu);
        }
        break;
    }
    case 6:
        run_targets(m, r, sub6a, 8u, 0x00C8u);
        if (run_steps_or(m, r, sub6b, 4u) == 0u)
            wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
        wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) - 0x00020000u);
        break;
    default: /* 7 */
        run_targets(m, r, sub7, 14u, 0x07D0u);
        {
            uint16_t d7 = 0u;
            if ((int16_t)rd16(m, F3_A5 + 0x4208u) > -0x0048) {
                wr16(m, F3_A5 + 0x4208u, (uint16_t)(rd16(m, F3_A5 + 0x4208u) - 2u));
                d7 |= 1u;
            }
            if ((int16_t)rd16(m, F3_A5 + 0x30D6u) > -0x0104) {
                wr16(m, F3_A5 + 0x30D6u, (uint16_t)(rd16(m, F3_A5 + 0x30D6u) - 0x0012u));
                d7 |= 2u;
            }
            if (d7 == 0u)
                wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
        }
        break;
    }
}

/* State 2 tick: ROM 0x133CFC. */
static void tick_state2(const f3mem_t *m, f3regs_t *r)
{
    static const pose_target_t tbl_a[14] = {
        { 0x32B6u, 0x9000u }, { 0x32B8u, 0x7800u }, { 0x32BAu, 0x1800u },
        { 0x32B0u, 0x9000u }, { 0x32B2u, 0x7800u }, { 0x32B4u, 0x1800u },
        { 0x32C0u, 0xA000u }, { 0x32C2u, 0x5800u },
        { 0x32BCu, 0xA000u }, { 0x32BEu, 0x5800u },
        { 0x32C8u, 0x9800u }, { 0x32CAu, 0x6000u },
        { 0x32C4u, 0x9800u }, { 0x32C6u, 0x6000u },
    };
    static const pose_target_t tbl_b[14] = {
        { 0x32B6u, 0x4000u }, { 0x32B8u, 0x4000u }, { 0x32BAu, 0x4000u },
        { 0x32B0u, 0x4000u }, { 0x32B2u, 0x4000u }, { 0x32B4u, 0x4000u },
        { 0x32C0u, 0x7000u }, { 0x32C2u, 0x4800u },
        { 0x32BCu, 0x7000u }, { 0x32BEu, 0x4800u },
        { 0x32C8u, 0x6000u }, { 0x32CAu, 0x3800u },
        { 0x32C4u, 0x6000u }, { 0x32C6u, 0x3800u },
    };
    uint32_t obj = r->a[0];
    if (rd8(m, obj + 0x22u) == 0u) {
        uint16_t d7 = 0u;
        for (uint32_t i = 0; i < 14u; i++)
            d7 |= step_1337AA(m, r, tbl_a[i].target, 0x0258u, F3_A5 + tbl_a[i].off);
        if ((int16_t)rd16(m, F3_A5 + 0x30D6u) < 0x0090) {
            wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + 0x00008000u);
            d7 = 0xFFFFu;
        }
        if (d7 == 0u) {
            wr8(m, obj + 0x22u, 0xFFu);
            Sound_PlayHooked(m, 0x0013u);
        }
    } else {
        run_targets(m, r, tbl_b, 14u, 0x07D0u);
        if ((int16_t)rd16(m, F3_A5 + 0x30D6u) > -0x0104) {
            wr16(m, F3_A5 + 0x30D6u, (uint16_t)(rd16(m, F3_A5 + 0x30D6u) - 0x0012u));
        } else {
            wr8(m, F3_A5 + 0x30F2u, 0u);
            wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u));
        }
    }
}

/* State 3 tick: ROM 0x133FA0. */
static void tick_state3(const f3mem_t *m, f3regs_t *r)
{
    static const pose_target_t tbl_a[4] = {
        { 0x32C0u, 0x9800u }, { 0x32BCu, 0x9800u },
        { 0x32C8u, 0x9000u }, { 0x32C4u, 0x9000u },
    };
    static const pose_target_t tbl_b[4] = {
        { 0x32C0u, 0xA000u }, { 0x32BCu, 0xA000u },
        { 0x32C8u, 0x9800u }, { 0x32C4u, 0x9800u },
    };
    uint32_t obj = r->a[0];
    if (rd8(m, obj + 0x22u) == 0u) {
        run_targets(m, r, tbl_a, 4u, 0x03E8u);
        if (rd8(m, obj + 0x23u) == 0u && (int16_t)rd16(m, F3_A5 + 0x30D6u) > 0x0060) {
            wr8(m, obj + 0x23u, 0xFFu);
            Sound_PlayHooked(m, 0x0025u);
            wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x20u));
        }
        int32_t d0 = 0x009C0000 - (int32_t)rd32(m, F3_A5 + 0x30D6u);
        uint32_t d1 = ((uint32_t)d0 << 16) | ((uint32_t)d0 >> 16);
        d0 = ASR32(d0, 3);
        wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + (uint32_t)d0);
        r->d[0] = (uint32_t)d0;
        r->d[1] = d1;
        if ((int16_t)d1 <= 0) {
            wr8(m, obj + 0x22u, 0xFFu);
            wr32(m, obj + 0x18u, 0u);
        }
    } else {
        run_targets(m, r, tbl_b, 4u, 0x0320u);
        uint32_t d0 = rd32(m, obj + 0x18u);
        wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) - d0);
        wr32(m, obj + 0x18u, d0 + 0x00002000u);
        r->d[0] = d0;
        if ((int16_t)rd16(m, obj + 0x18u) >= 3) {
            wr8(m, F3_A5 + 0x30F2u, 0u);
            wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u));
        }
    }
}

/* State 4 tick: ROM 0x1340AE. */
static void tick_state4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t p = rd32(m, F3_A5 + 0x1D28u);
    uint16_t dx = (uint16_t)(rd16(m, p + 0x14u) + 0x0018u - rd16(m, F3_A5 + 0x30D2u) + 0x0017u);
    uint16_t dy = (uint16_t)(rd16(m, p + 0x18u) + 0x0010u - rd16(m, F3_A5 + 0x30D6u) - 0x000Fu);
    uint16_t ang = (uint16_t)(Obj_Atan2(m, dx, dy) << 8);

    if (rd16(m, obj + 0x3Au) == 0u) {
        step_1337CE(m, r, ang, 0x07D0u, F3_A5 + 0x32B6u);
        step_1337CE(m, r, ang, 0x07D0u, F3_A5 + 0x32B8u);
        (void)step_1337AA(m, r, ang, 0x07D0u, F3_A5 + 0x32BAu);
    } else {
        uint16_t neg_ang = (uint16_t)(0u - ang);
        step_1337C0(m, r, neg_ang, 0x0190u, F3_A5 + 0x32B6u);
        step_1337C0(m, r, neg_ang, 0x0190u, F3_A5 + 0x32B8u);
        (void)step_1337AA(m, r, ang, 0x0190u, F3_A5 + 0x32BAu);
    }

    p = rd32(m, F3_A5 + 0x1D28u);
    dx = (uint16_t)(rd16(m, p + 0x14u) + 0x0018u - rd16(m, F3_A5 + 0x30D2u) - 0x0017u);
    dy = (uint16_t)(rd16(m, p + 0x18u) + 0x0010u - rd16(m, F3_A5 + 0x30D6u) - 0x000Fu);
    ang = (uint16_t)(Obj_Atan2(m, dx, dy) << 8);
    ang = (uint16_t)(0x4000u - (uint16_t)(ang - 0x4000u));

    if (rd16(m, obj + 0x3Au) != 0u) {
        step_1337CE(m, r, ang, 0x07D0u, F3_A5 + 0x32B0u);
        step_1337CE(m, r, ang, 0x07D0u, F3_A5 + 0x32B2u);
        (void)step_1337AA(m, r, ang, 0x07D0u, F3_A5 + 0x32B4u);
    } else {
        uint16_t neg_ang = (uint16_t)(0u - ang);
        step_1337C0(m, r, neg_ang, 0x0190u, F3_A5 + 0x32B0u);
        step_1337C0(m, r, neg_ang, 0x0190u, F3_A5 + 0x32B2u);
        (void)step_1337AA(m, r, ang, 0x0190u, F3_A5 + 0x32B4u);
    }

    if (rd16(m, obj + 0x3Eu) == 0u)
        Sound_PlayHooked(m, 0x002Bu);
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
    wr16(m, obj + 0x3Eu, cnt);
    if ((int16_t)cnt >= 0x003C) {
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)~rd16(m, obj + 0x3Au));
    }
}

/* State 5 tick: ROM 0x134212. */
static void tick_state5(const f3mem_t *m, f3regs_t *r)
{
    static const pose_target_t tbl[14] = {
        { 0x32B6u, 0x9000u }, { 0x32B8u, 0x6800u }, { 0x32BAu, 0x1800u },
        { 0x32B0u, 0x9000u }, { 0x32B2u, 0x6800u }, { 0x32B4u, 0x1800u },
        { 0x32C0u, 0x9000u }, { 0x32C2u, 0x5800u },
        { 0x32BCu, 0x9000u }, { 0x32BEu, 0x5800u },
        { 0x32C8u, 0x9800u }, { 0x32CAu, 0x4000u },
        { 0x32C4u, 0x9800u }, { 0x32C6u, 0x4000u },
    };
    run_targets(m, r, tbl, 14u, 0x0258u);
}

/* State 6 tick: ROM 0x1342FA. */
static void tick_state6(const f3mem_t *m, f3regs_t *r)
{
    static const pose_target_t tbl[6] = {
        { 0x32B6u, 0x8800u }, { 0x32B0u, 0x8800u },
        { 0x32B8u, 0x7000u }, { 0x32B2u, 0x7000u },
        { 0x32BAu, 0x1800u }, { 0x32B4u, 0x1800u },
    };
    run_targets(m, r, tbl, 6u, 0x0190u);
    sub_13459A(m, r);
}

/* State 7 tick: ROM 0x13436E. */
static void tick_state7(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = 0x001343FAu;
    uint16_t d2 = 0x07D0u;
    uint32_t a3 = 0x00134400u;
    uint16_t d3 = 0x0190u;
    if (rd16(m, obj + 0x3Au) != 0u) {
        a2 = 0x00134400u; d2 = 0x0190u;
        a3 = 0x001343FAu; d3 = 0x07D0u;
    }
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
    wr16(m, obj + 0x3Eu, cnt);
    if ((int16_t)cnt >= 0x003C) {
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)~rd16(m, obj + 0x3Au));
    }
    (void)step_1337AA(m, r, rd16(m, a2 + 0u), d2, F3_A5 + 0x32B6u);
    (void)step_1337AA(m, r, rd16(m, a2 + 2u), d2, F3_A5 + 0x32B8u);
    (void)step_1337AA(m, r, rd16(m, a2 + 4u), d2, F3_A5 + 0x32BAu);
    (void)step_1337AA(m, r, rd16(m, a3 + 0u), d3, F3_A5 + 0x32B0u);
    (void)step_1337AA(m, r, rd16(m, a3 + 2u), d3, F3_A5 + 0x32B2u);
    (void)step_1337AA(m, r, rd16(m, a3 + 4u), d3, F3_A5 + 0x32B4u);
    sub_13459A(m, r);
}

/* State 8 tick: ROM 0x134418. */
static void tick_state8(const f3mem_t *m, f3regs_t *r)
{
    static const pose_target_t tbl_a[14] = {
        { 0x32B6u, 0x8800u }, { 0x32B0u, 0x8800u },
        { 0x32B8u, 0x9000u }, { 0x32B2u, 0x9000u },
        { 0x32BAu, 0x0800u }, { 0x32B4u, 0x0800u },
        { 0x32C0u, 0x8800u }, { 0x32BCu, 0x8800u },
        { 0x32C2u, 0x5000u }, { 0x32BEu, 0x5000u },
        { 0x32C8u, 0x9000u }, { 0x32C4u, 0x9000u },
        { 0x32CAu, 0x5800u }, { 0x32C6u, 0x5800u },
    };
    static const pose_target_t tbl_b[6] = {
        { 0x32B6u, 0x9000u }, { 0x32B0u, 0x9000u },
        { 0x32B8u, 0x9000u }, { 0x32B2u, 0x9000u },
        { 0x32BAu, 0x6000u }, { 0x32B4u, 0x6000u },
    };
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)rd16(m, obj + 0x3Eu);
    if (cnt < 0x00F0) {
        wr16(m, obj + 0x3Eu, (uint16_t)(cnt + 1));
        run_targets(m, r, tbl_a, 14u, 0x0190u);
    } else {
        run_targets(m, r, tbl_b, 6u, 0x0FA0u);
        uint8_t b = rd8(m, F3_A5 + 0x30EEu);
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(b & ~0x02u));
        if ((b & 0x02u) == 0u)
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x01u));
    }
}

int Fn_133720(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x133720: bfclr (0x30ee,A5){0xc:1} */
    uint8_t b = rd8(m, F3_A5 + 0x30EFu);
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(b & ~0x08u));
    if ((b & 0x08u) != 0u) {
        uint8_t st = rd8(m, F3_A5 + 0x30F2u);
        switch (st) {
        case 0u: break;                                 /* 0x1337E8 rts */
        case 1u: wr16(m, obj + 0x22u, 0u); break;       /* 0x1338C4 */
        case 2u: wr8(m, obj + 0x22u, 0u); break;        /* 0x133CF6 */
        case 3u:                                        /* 0x133F42 */
            wr16(m, F3_A5 + 0x32B6u, 0x7800u);
            wr16(m, F3_A5 + 0x32B0u, 0x7800u);
            wr16(m, F3_A5 + 0x32B8u, 0x7800u);
            wr16(m, F3_A5 + 0x32B2u, 0x7800u);
            wr16(m, F3_A5 + 0x32BAu, 0x1800u);
            wr16(m, F3_A5 + 0x32B4u, 0x1800u);
            wr16(m, F3_A5 + 0x32C0u, 0x6800u);
            wr16(m, F3_A5 + 0x32BCu, 0x6800u);
            wr16(m, F3_A5 + 0x32C2u, 0x6000u);
            wr16(m, F3_A5 + 0x32BEu, 0x6000u);
            wr16(m, F3_A5 + 0x32C8u, 0x7000u);
            wr16(m, F3_A5 + 0x32C4u, 0x7000u);
            wr16(m, F3_A5 + 0x32CAu, 0x6800u);
            wr16(m, F3_A5 + 0x32C6u, 0x6800u);
            wr8(m, obj + 0x22u, 0u);
            wr8(m, obj + 0x23u, 0u);
            break;
        case 4u:                                        /* 0x1340A4 */
            wr16(m, obj + 0x3Au, 0u);
            wr16(m, obj + 0x3Eu, 0u);
            break;
        case 5u: break;                                 /* 0x134210 rts */
        case 6u: sub_13457C(m, obj); break;             /* 0x1342F4 */
        case 7u:                                        /* 0x134360 */
            sub_13457C(m, obj);
            wr16(m, obj + 0x3Au, 0u);
            wr16(m, obj + 0x3Eu, 0u);
            break;
        case 8u:                                        /* 0x134406 */
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x02u));
            wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x01u));
            wr16(m, obj + 0x3Eu, 0u);
            break;
        default:
            return -1;
        }
    }

    /* 0x13373A */
    uint8_t st = rd8(m, F3_A5 + 0x30F2u);
    r->d[0] = (uint32_t)st;
    switch (st) {
    case 0u: tick_state0(m, r); break;
    case 1u: tick_state1(m, r); break;
    case 2u: tick_state2(m, r); break;
    case 3u: tick_state3(m, r); break;
    case 4u: tick_state4(m, r); break;
    case 5u: tick_state5(m, r); break;
    case 6u: tick_state6(m, r); break;
    case 7u: tick_state7(m, r); break;
    case 8u: tick_state8(m, r); break;
    default: return -1;
    }

    uint32_t parent = rd32(m, obj + 0x08u);             /* 0x13374A */
    r->a[1] = parent;
    if (rd8(m, parent + 0x59u) == 0u)                   /* 0x13374E */
        wr32(m, obj, 0x00133720u);                      /* 0x133718 */
    else
        Obj_Free(m, obj);                               /* 0x133754 */
    return 0;
}
