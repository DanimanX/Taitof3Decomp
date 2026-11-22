#include "obj_17AAFE.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))


/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d1w * (uint32_t)d0w) << 8;
    d1w = (uint16_t)(d1 >> 16);
    if (d1w == 0u)
        d1w = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % d1w);
}

/* ROM 0x0CBB0C: warp target when bit 2 of A5+0x1D58 is set */
static void __attribute__((unused)) sub_0CBB0C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x04u) {
        wr8(m, obj + 0x35u, 0x01u);
        wr32(m, obj + 0x36u, rd32(m, F3_A5 + 0x1D54u));
        wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) - 0x4000u);
    }
}


/* A5+0x4B3A boss flags: bit n counted from the MSB (bftst {n:1}) */
static int boss_flag(const f3mem_t *m, unsigned n)
{
    return (rd8(m, F3_A5 + 0x4B3Au + n / 8u) >> (7u - n % 8u)) & 1u;
}

static void boss_flag_clr(const f3mem_t *m, unsigned n)
{
    uint32_t a = F3_A5 + 0x4B3Au + n / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) & ~(1u << (7u - n % 8u))));
}

/* ROM 0x1794A4: D0w = random in [-(d0-1)..d0-1]-ish via LCG at A5+0x4B3C */
static uint16_t sub_1794A4(const f3mem_t *m, uint32_t d0)
{
    uint16_t r = (uint16_t)d0;
    if (d0 != 0u) {
        int off = 0;
        while (!(d0 & (0x80000000u >> off)))
            off++;
        int16_t d1w = (int16_t)(off - 16);
        uint16_t d2w = rd16(m, 0x179530u + (uint32_t)((int32_t)d1w * 2));
        uint16_t v = (uint16_t)(rd16(m, F3_A5 + 0x4B3Cu) & d2w);
        if ((int16_t)r < (int16_t)v)
            v = (uint16_t)(v - r);
        r = v;
        if (rd8(m, F3_A5 + 0x4B3Cu) & 1u)
            r = (uint16_t)(0u - r);
    }
    uint32_t s = rd32(m, F3_A5 + 0x4B3Cu) * 5u - 1u;
    wr32(m, F3_A5 + 0x4B3Cu, (s << 16) | (s >> 16));
    return r;
}

/* ---- frame script engine, op table 0x179A5C {init, tick} ---- */
static void scr_ofs(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 1u)));
}

/* timed frame record at +0x3A: dur.w spr.l [1c.w] [2c.l 30.l] [dx.b dy.b] */
static void scr_frame(const f3mem_t *m, uint32_t obj, int has1c, int hasbox, int hasofs)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    if (has1c) { wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u; }
    if (hasbox) {
        wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    }
    if (hasofs)
        scr_ofs(m, obj, a1);
}

/* directional record selected by the angle byte at +0x20 */
static void scr_dir(const f3mem_t *m, uint32_t obj, int fine, uint32_t size,
                    int has1c, int hasbox, int hasofs)
{
    uint8_t b = rd8(m, obj + 0x20u);
    uint16_t idx = fine ? (uint16_t)((uint8_t)(b + 4u) >> 3) : (uint16_t)((uint8_t)(b + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + idx * size;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    if (has1c) { wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u; }
    if (hasbox) {
        wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    }
    if (hasofs)
        scr_ofs(m, obj, a1);
}

static void scr_dir16(const f3mem_t *m, uint32_t obj)   /* ROM 0x179A1E */
{
    int8_t b = (int8_t)(uint8_t)(rd8(m, obj + 0x20u) - 0x20u);
    if (b < 0) b = 0;
    if (b > 0x40) b = 0x40;
    uint16_t idx = (uint16_t)((uint8_t)(b + 2) >> 2);
    uint32_t a1 = rd32(m, obj + 0x3Au) + idx * 6u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
}

static int scr_tick(const f3mem_t *m, uint32_t obj, uint16_t op);

static int scr_init(const f3mem_t *m, uint32_t obj, uint16_t op)
{
    switch (op) {
    case 0:  scr_frame(m, obj, 0, 0, 0); return 0;
    case 8:  scr_frame(m, obj, 1, 0, 0); return 0;
    case 3:  scr_frame(m, obj, 0, 1, 0); return 0;
    case 11: scr_frame(m, obj, 1, 1, 0); return 0;
    case 5:  scr_frame(m, obj, 0, 1, 1); return 0;
    case 13: scr_frame(m, obj, 1, 1, 1); return 0;
    case 1: case 2: case 4: case 6: case 7: case 9: case 10:
    case 12: case 14: case 15: case 16:
        wr16(m, obj + 0x3Eu, 1u);
        return scr_tick(m, obj, op);
    default:
        return -1;
    }
}

/* jump: A1 -> {op.w, data...} */
static int scr_jump(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t op = rd16(m, a1);
    wr16(m, obj + 0x40u, op);
    wr32(m, obj + 0x3Au, a1 + 2u);
    return scr_init(m, obj, op);
}

static int scr_timed(const f3mem_t *m, uint32_t obj, uint16_t op, uint32_t step, int ofs)
{
    int16_t t = (int16_t)rd16(m, obj + 0x3Eu);
    if (t == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + step;
        int16_t w = (int16_t)rd16(m, a1);
        if (w > 0) {
            wr32(m, obj + 0x3Au, a1);
            scr_init(m, obj, op);
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        }
        if (w == 0) {
            int r = scr_jump(m, obj, rd32(m, a1 + 2u));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return r;
        }
        t = 1; /* w < 0: hold the frame */
    }
    if (t > 0)
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    else if (!ofs)
        return 0;
    if (ofs)
        scr_ofs(m, obj, rd32(m, obj + 0x3Au) + step - 2u);
    return 0;
}

static int scr_tick(const f3mem_t *m, uint32_t obj, uint16_t op)
{
    switch (op) {
    case 0:  return scr_timed(m, obj, 0, 6u, 0);
    case 8:  return scr_timed(m, obj, 8, 8u, 0);
    case 3:  return scr_timed(m, obj, 3, 0x0Eu, 0);
    case 11: return scr_timed(m, obj, 11, 0x10u, 0);
    case 5:  return scr_timed(m, obj, 5, 0x10u, 1);
    case 13: return scr_timed(m, obj, 13, 0x12u, 1);
    case 1:  scr_dir(m, obj, 0, 4u, 0, 0, 0); return 0;
    case 9:  scr_dir(m, obj, 0, 6u, 1, 0, 0); return 0;
    case 2:  scr_dir(m, obj, 1, 4u, 0, 0, 0); return 0;
    case 10: scr_dir(m, obj, 1, 6u, 1, 0, 0); return 0;
    case 4:  scr_dir(m, obj, 0, 0x0Cu, 0, 1, 0); return 0;
    case 12: scr_dir(m, obj, 0, 0x0Eu, 1, 1, 0); return 0;
    case 6:  scr_dir(m, obj, 0, 0x0Eu, 0, 1, 1); return 0;
    case 14: scr_dir(m, obj, 0, 0x10u, 1, 1, 1); return 0;
    case 7:  scr_dir(m, obj, 1, 0x0Eu, 0, 1, 1); return 0;
    case 15: scr_dir(m, obj, 1, 0x10u, 1, 1, 1); return 0;
    case 16: scr_dir16(m, obj); return 0;
    default: return -1;
    }
}

/* ROM 0x17BA76: hit point between obj and the attacker at +0x36 (D0w < 0: none) */
static void sub_17BA76(const f3mem_t *m, uint32_t obj, uint16_t *o0, uint16_t *o1)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 2u) {
        if ((rd16(m, F3_A5 + 0x878u) % 3u) != 0u) { *o0 = 0xFFFFu; return; }
        uint16_t d2 = (uint16_t)ASR16((uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)), 1);
        uint16_t d3 = (uint16_t)ASR16((uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u)), 1);
        uint16_t r = sub_1794A4(m, d3);
        *o1 = (uint16_t)(r + rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u) + d3);
        r = sub_1794A4(m, d2);
        *o0 = (uint16_t)(r + rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu) + d2);
        return;
    }
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u && (rd16(m, F3_A5 + 0x878u) & 3u) != 0u) { *o0 = 0xFFFFu; return; }
    int16_t d0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu));
    int16_t d1 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u));
    int16_t d2 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Cu));
    int16_t d3 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x30u));
    int16_t d4 = d2 > d0 ? d2 : d0, d5 = d3 > d1 ? d3 : d1;
    d0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Eu));
    d1 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x32u));
    d2 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Eu));
    d3 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x32u));
    int16_t d6 = d2 < d0 ? d2 : d0, d7 = d3 < d1 ? d3 : d1;
    *o0 = (uint16_t)(d4 + ((uint16_t)(d6 - d4) >> 1));
    *o1 = (uint16_t)(d5 + ((uint16_t)(d7 - d5) >> 1));
}

static void hit_spark(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    uint16_t d0, d1 = 0;
    sub_17BA76(m, obj, &d0, &d1);
    if ((int16_t)d0 < 0)
        return;
    d0 = (uint16_t)(d0 - rd16(m, obj + 0x50u));
    d1 = (uint16_t)(d1 - rd16(m, obj + 0x54u));
    Obj_Spawn(m, obj, fn, 5u, 2u, ((uint32_t)d0 << 16) | d1);
}

/* ROM 0x17B9BA */
static void sub_17B9BA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    if (sub_1794A4(m, 5u) != 0u)
        return;
    hit_spark(m, obj, 0x17A122u);
}

/* ---- boss part behaviours (vtable +8) ---- */
static void part_follow(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);
}

static void part_sprofs(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = rd16(m, a1), d0 = rd16(m, a1 + 2u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(d0 << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(d1 << 3)));
}

static void sub_188EE8(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, rd32(m, obj + 0x0Cu) + 4u);
    wr16(m, obj + 0x16u, rd16(m, a1 + 0x0Eu));
    wr16(m, obj + 0x1Au, rd16(m, a1 + 0x10u));
    wr8(m, obj + 0x1Cu, rd8(m, a1 + 0x16u));
    wr32(m, obj + 0x2Cu, rd32(m, obj + 0x58u));
    wr32(m, obj + 0x30u, rd32(m, obj + 0x68u));
    if (boss_flag(m, 14)) {
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 1u));
        uint16_t d0 = (uint16_t)((rd16(m, rd32(m, obj + 0x10u) + 2u) << 4) - 1u);
        uint16_t d1 = d0;
        d0 = (uint16_t)(d0 - rd16(m, obj + 0x58u));
        d1 = (uint16_t)(d1 - rd16(m, obj + 0x5Au));
        wr16(m, obj + 0x2Cu, d1);
        wr16(m, obj + 0x2Eu, d0);
        wr16(m, obj + 0x16u, (uint16_t)(0x10u - rd16(m, obj + 0x16u)));
    }
    part_follow(m, obj);
    part_sprofs(m, obj);
}

static void sub_188F84(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x2Cu, 0u);
    wr32(m, obj + 0x30u, 0u);
    wr16(m, obj + 0x16u, 0u);
    wr16(m, obj + 0x1Au, 0u);
    wr32(m, obj + 0x10u, 0x28740u);
    part_follow(m, obj);
}

/* ROM 0x188E32: common tail */
static int part_tail(const f3mem_t *m, uint32_t obj)
{
    if (boss_flag(m, 13)) {
        sub_188F84(m, obj);
        return 0;
    }
    if (rd32(m, obj + 0x10u) == 0x28740u &&
        scr_jump(m, obj, rd32(m, rd32(m, rd32(m, obj + 0x0Cu) + 4u) + 0x12u)) < 0)
        return -1;
    sub_188EE8(m, obj);
    return 0;
}

/* ROM 0x188B40 */
static void sub_188B40(const f3mem_t *m, uint32_t obj, int16_t d0, uint32_t a1)
{
    uint32_t key = rd32(m, obj + 0x10u);
    for (;;) {
        uint32_t k = rd32(m, a1);
        a1 += 4u;
        if (k == key)
            break;
        a1 += 8u;
        if (--d0 < 0)
            break;
    }
    wr32(m, obj + 0x58u, rd32(m, a1));
    wr32(m, obj + 0x68u, rd32(m, a1 + 4u));
}

/* ROM 0x1888A2 */
static void sub_1888A2(const f3mem_t *m, uint32_t obj)
{
    if (sub_1794A4(m, 0x14u) != 0u)
        return;
    uint32_t d1 = (uint32_t)sub_1794A4(m, 0x10u) << 16;
    d1 |= sub_1794A4(m, 0x10u);
    Obj_Spawn(m, obj, (rd8(m, F3_A5 - 0x1456u) & 1u) ? 0x179DCCu : 0x179D26u, 3u, 2u, d1);
}

static uint8_t sub_17AF9A(const f3mem_t *m, uint32_t p, uint16_t x, uint16_t y)
{
    if ((int32_t)p <= 0)
        return (uint8_t)p;
    uint16_t dx = (uint16_t)(rd16(m, p + 0x14u) + 0x18u - x);
    uint16_t dy = (uint16_t)(rd16(m, p + 0x18u) + 0x10u - y);
    return (uint8_t)sub_006422(m, dx, dy);
}

static int part_188758(const f3mem_t *m, uint32_t obj)
{
    int cleared = 1;
    if (boss_flag(m, 4)) {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x64u) - 1u);
        wr16(m, obj + 0x64u, t);
        if ((int16_t)t >= 0) {
            cleared = 0;
            if (sub_0CC2BC(m, 8u) == 0u) {
                uint32_t a1 = boss_flag(m, 14) ? 0x188888u : 0x188880u;
                for (int i = 0; i < 2; i++, a1 += 4u)
                    Obj_Spawn(m, obj, 0x17E000u, 9u, 2u, rd32(m, a1));
            }
        }
    }
    if (cleared) {
        boss_flag_clr(m, 4);
        wr16(m, obj + 0x64u, 0x14u);
    }
    if (boss_flag(m, 3)) {
        wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x62u) + 0x100u));
        if ((int8_t)rd8(m, obj + 0x62u) < 0x18 && (int16_t)rd16(m, obj + 0x4Cu) > 0) {
            if ((rd8(m, obj + 0x62u) & 3u) == 0u) {
                uint32_t d1 = boss_flag(m, 14) ? 0x03FC0000u : 0xFCFC0000u;
                d1 = (d1 & ~0xFFu) | (uint8_t)(rd8(m, obj + 0x20u) + rd8(m, obj + 0x62u));
                Obj_Spawn(m, obj, 0x184170u, 9u, 2u, d1);
                d1 = (d1 & ~0xFFu) | (uint8_t)(rd8(m, obj + 0x20u) - rd8(m, obj + 0x62u) - 1u);
                Obj_Spawn(m, obj, 0x184170u, 9u, 2u, d1);
            }
            return part_tail(m, obj);
        }
    }
    boss_flag_clr(m, 3);
    wr16(m, obj + 0x62u, 0u);
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
    wr8(m, obj + 0x20u, sub_17AF9A(m, rd32(m, obj + 0x4Cu), rd16(m, obj + 0x50u), rd16(m, obj + 0x54u)));
    return part_tail(m, obj);
}

static int part_188890(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) >= 3)
        sub_1888A2(m, obj);
    return part_tail(m, obj);
}

static int part_18890A(const f3mem_t *m, uint32_t obj)
{
    if (boss_flag(m, 5) && sub_0CC2BC(m, 8u) == 0u) {
        uint32_t d1 = boss_flag(m, 14) ? 0x04000080u : 0xFB000000u;
        uint8_t r = (uint8_t)sub_1794A4(m, 8u);
        d1 = (d1 & ~0xFFu) | (uint8_t)(d1 + r);
        Obj_Spawn(m, obj, 0x17D136u, 9u, 2u, d1);
    }
    if (boss_flag(m, 6) && sub_0CC2BC(m, 0x50u) == 0u) {
        uint32_t d1 = boss_flag(m, 14) ? 0x06000080u : 0xFB000000u;
        Obj_Spawn(m, obj, 0x17E4F0u, 9u, 2u, d1);
    }
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) >= 5)
        sub_1888A2(m, obj);
    return part_tail(m, obj);
}

static int part_1889EA(const f3mem_t *m, uint32_t obj, uint32_t a2)
{
    uint32_t key = rd32(m, obj + 0x10u);
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        uint32_t a1 = 0;
        if (boss_flag(m, 7)) {
            if (rd32(m, a2) == key) a1 = rd32(m, a2 + 8u);
        } else {
            if (rd32(m, a2 + 4u) == key) a1 = rd32(m, a2 + 0x0Cu);
        }
        if (a1 && scr_jump(m, obj, a1) < 0)
            return -1;
    }
    if (boss_flag(m, 8) && rd32(m, a2 + 4u) == rd32(m, obj + 0x10u)) {
        uint32_t a1 = rd32(m, a2 + (boss_flag(m, 14) ? 0x14u : 0x10u));
        for (int d7 = 5; d7 >= 0; d7--) {
            uint32_t d0 = rd32(m, a1 + (uint32_t)d7 * 4u);
            Obj_Spawn(m, obj, boss_flag(m, 9) ? 0x17E188u : 0x17E33Cu, 9u, 2u, d0);
        }
    }
    return part_tail(m, obj);
}

static int part_188CB8(const f3mem_t *m, uint32_t obj)
{
    uint32_t k = rd32(m, obj + 0x10u);
    wr8(m, obj + 0x34u, (k == 0x33E40u || k == 0x28740u) ? 0x0Au : 0x10u);
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        uint32_t a1 = 0;
        if (boss_flag(m, 1)) {
            if (k == 0x33E40u) a1 = 0x189292u;
        } else {
            if (k == 0x33F0Cu) a1 = 0x1892AEu;
        }
        if (a1 && scr_jump(m, obj, a1) < 0)
            return -1;
    }
    if (rd32(m, obj + 0x10u) == 0x33F0Cu && boss_flag(m, 2) && sub_0CC2BC(m, 0x14u) == 0u) {
        uint32_t d1 = boss_flag(m, 14) ? 0x08F00000u : 0xF7F00080u;
        static const int8_t step[4] = { 0x10, 0x10, -0x30, -0x10 };
        for (int i = 0; i < 4; i++) {
            d1 = (d1 & ~0xFFu) | (uint8_t)(d1 + (uint8_t)step[i]);
            Obj_Spawn(m, obj, 0x184184u, 9u, 2u, d1);
        }
    }
    return part_tail(m, obj);
}

static int part_188E84(const f3mem_t *m, uint32_t obj)
{
    if (!boss_flag(m, 13)) {
        sub_188F84(m, obj);
        return 0;
    }
    if (boss_flag(m, 0)) {
        if (scr_jump(m, obj, rd32(m, rd32(m, rd32(m, obj + 0x0Cu) + 4u) + 0x12u)) < 0)
            return -1;
        wr16(m, obj + 0x62u, 0x28u);
    }
    sub_188EE8(m, obj);
    uint16_t t = (uint16_t)(rd16(m, obj + 0x62u) - 1u);
    wr16(m, obj + 0x62u, t);
    if ((int16_t)t < 0 && rd32(m, obj + 0x0Cu) == 0x189022u)
        boss_flag_clr(m, 13);
    return 0;
}

static int part_known(uint32_t fn)
{
    switch (fn) {
    case 0x188758u: case 0x188890u: case 0x18890Au: case 0x1889BAu: case 0x1889D2u:
    case 0x188CB8u: case 0x188E78u: case 0x188E84u:
        return 1;
    default:
        return 0;
    }
}

static int part_call(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    switch (fn) {
    case 0x188758u: return part_188758(m, obj);
    case 0x188890u: return part_188890(m, obj);
    case 0x18890Au: return part_18890A(m, obj);
    case 0x1889BAu: sub_188B40(m, obj, 0x0C, 0x188B5Cu); return part_1889EA(m, obj, 0x188AB0u);
    case 0x1889D2u: sub_188B40(m, obj, 0x09, 0x188BF8u); return part_1889EA(m, obj, 0x188AC8u);
    case 0x188CB8u: return part_188CB8(m, obj);
    case 0x188E78u: sub_188B40(m, obj, 0x05, 0x188C70u); return part_188E84(m, obj);
    case 0x188E84u: return part_188E84(m, obj);
    default: return -1;
    }
}

/* ROM 0x17A8E0 / 0x17AB34 body: follow parent, run script, part behaviour */
static int part_body(const f3mem_t *m, uint32_t obj)
{
    part_follow(m, obj);
    if (scr_tick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    part_sprofs(m, obj);
    return 0;
}

static void part_lists(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x906u, a1 + 4u);
}

/* ROM 0x17B92C: pass damage taken to the parent (+8) */
static void sub_17B92C(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 0u)
        return;
    int32_t d0 = (int32_t)rd32(m, obj + 0x24u);
    uint32_t mul = 0;
    if (k == 2u) {
        mul = F3_A5 + 0x4B2Eu;
    } else {
        uint8_t t = rd8(m, rd32(m, obj + 0x36u) + 0x22u);
        if (t == 1u) mul = F3_A5 + 0x4B2Au;
        else if (t == 2u) mul = F3_A5 + 0x4B2Cu;
    }
    if (mul)
        d0 = (int32_t)((uint32_t)(d0 / 100) * (uint32_t)rd16(m, mul));
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr32(m, a4 + 0x24u, rd32(m, a4 + 0x24u) + (uint32_t)d0);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
    wr16(m, a4 + 0x34u, rd16(m, obj + 0x34u));
}

/* ROM 0x17AAFE: boss part tick (damage forwarded to the parent) */
int Obj_17AAFE_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu || (int32_t)rd32(m, obj + 0x28u) < 0) {
        wr16(m, obj + 0x06u, 2u);
        wr32(m, obj, 0x17ABC8u);
        return 0;
    }
    uint32_t vf = rd32(m, rd32(m, obj + 0x0Cu) + 8u);
    if (!part_known(vf))
        return -1;
    if (rd8(m, obj + 0x34u) & 0x02u) {
        sub_17B9BA(m, obj);
        wr8(m, obj + 0x35u, 0u);
        sub_0CBB0C(m, obj);
    } else if (rd8(m, obj + 0x35u) != 0u) {
        hit_spark(m, obj, 0x17A016u);     /* ROM 0x17BA02 */
    }
    sub_17B92C(m, obj);
    if (part_body(m, obj) < 0)
        return -1;
    if (part_call(m, obj, vf) < 0)
        return -1;
    if (!boss_flag(m, 15))
        part_lists(m, obj);
    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    wr8(m, obj + 0x35u, 0u);
    Obj_DrawSprite(m, obj);
    wr32(m, obj, 0x17AAFEu);
    return 0;
}
