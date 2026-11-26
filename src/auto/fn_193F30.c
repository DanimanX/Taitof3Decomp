/* ROM 0x193F30: multi-joint boss tentacle segment tick (A0 = object).
 * Frees self + shadow (+0x58) via 0x0105EC when A5+0x4B38 == -1 or when
 * +0x24 <= 0 (awarding 0x7530 via 0x17B02E + 0x12C5F to A5+0x886, setting
 * bit +0x22 in A5+0x4B96, spawning 0x179DEE, playing sound 0x16, and
 * latching A5+0x4B26/4B20/4B1C if +0x44 <= 0). While active, switches
 * between extended/aimed attack (when A5+0x4B3A bit +0x22 is set: initializes
 * script10 from A5+0x4B98[+0x22] and anim from (+0x0C)->+0x04->+0x18,
 * steps 0x178982 or jitters on hit, ramps spin rate +0x64 up to 0x800,
 * plays sound 0x2B every 32 frames) and retracted idle (re-arming 0x19476C,
 * tracking parent +0x08, ramping +0x64 down to 0x400 to snap back to idle
 * anim (+0x0C)->+0x04->+0x12 with sound 0x3A and 4x 0x17A1DE). Pushes
 * collision lists when A5+0x4B3A bit 15 clear, steps 0x179A5C animation
 * (including index 13), records trailing sprite ring buffer (0x193DB6),
 * and draws either single sprite (index 13) or 4-ghost trail (0x193DE2)
 * with hit-feedback (0x193E6C). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
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

static int bf_test_mem(const f3mem_t *m, uint32_t base, uint32_t bit)
{
    return (rd8(m, base + (bit >> 3)) & (0x80u >> (bit & 7u))) != 0u;
}

static void bf_set_mem(const f3mem_t *m, uint32_t base, uint32_t bit)
{
    uint32_t a = base + (bit >> 3);
    wr8(m, a, (uint8_t)(rd8(m, a) | (0x80u >> (bit & 7u))));
}

/* ROM 0x1795CA: index-13 anim init (18-byte record with hitbox + dx.b/dy.b). */
static void anim13_init(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    int16_t dx = (int16_t)(int8_t)rd8(m, a1);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 1u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
}

static int anim_init_local(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (idx == 13u) {
        anim13_init(m, obj);
        return 0;
    }
    return Obj_AnimInit(m, obj, idx);
}

/* ROM 0x17980E: index-13 anim tick. */
static int anim13_tick(const f3mem_t *m, uint32_t obj)
{
    int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);
    if (timer == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + 0x12u;
        int16_t d = (int16_t)rd16(m, a1);
        if (d > 0) {
            wr32(m, obj + 0x3Au, a1);
            anim13_init(m, obj);
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        } else if (d == 0) {
            a1 = rd32(m, a1 + 2u);
            uint16_t next_idx = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, next_idx);
            wr32(m, obj + 0x3Au, a1);
            if (anim_init_local(m, obj, next_idx) < 0)
                return -1;
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
            return 0;
        }
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
    } else if (timer > 0) {
        wr16(m, obj + 0x3Eu, (uint16_t)(timer - 1));
    }
    uint32_t a1 = rd32(m, obj + 0x3Au);
    int16_t dx = (int16_t)(int8_t)rd8(m, a1 + 0x10u);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 0x11u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
    return 0;
}

static int anim_tick_local(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (idx == 13u)
        return anim13_tick(m, obj);
    return Obj_AnimTick(m, obj, idx);
}

/* ROM 0x178B02: set +0x20.w = arg0.w. */
static void scr_178B02(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x20u, rd16(m, rd32(m, obj + 0x44u) + 8u));
}

/* ROM 0x178B0E: set +0x42.w = arg0, +0x20.w = arg1. */
static void scr_178B0E(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x42u, rd16(m, a1 + 6u));
    wr16(m, obj + 0x20u, rd16(m, a1 + 8u));
}

/* ROM 0x178B7A: set (+0x4C, +0x4E) target minus one step if +0x48 > 0. */
static void scr_178B7A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    int16_t d0 = (int16_t)rd16(m, a1 + 6u);
    int16_t d1 = (int16_t)rd16(m, a1 + 8u);
    wr16(m, obj + 0x4Cu, (uint16_t)d0);
    wr16(m, obj + 0x4Eu, (uint16_t)d1);
    int32_t x = (int16_t)(uint16_t)(d0 - (int16_t)rd16(m, obj + 0x50u));
    int32_t y = (int16_t)(uint16_t)(d1 - (int16_t)rd16(m, obj + 0x54u));
    int16_t n = (int16_t)rd16(m, obj + 0x48u);
    if (n > 0) {
        wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, obj + 0x4Cu) - (uint16_t)(x / n)));
        wr16(m, obj + 0x4Eu, (uint16_t)(rd16(m, obj + 0x4Eu) - (uint16_t)(y / n)));
    }
    wr16(m, obj + 0x48u, 0u);
}

/* ROM 0x178BB4: set +0x4C = A5+0x1D28 (current player target). */
static void scr_178BB4(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
}

/* ROM 0x178C06: add (arg0, arg1) to (+0x42, +0x20) and step 0x27E40. */
static void scr_178C06(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    uint16_t d2 = (uint16_t)(rd16(m, obj + 0x20u) + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, d0);
    wr16(m, obj + 0x20u, d2);
    int16_t mag = (int16_t)(uint16_t)(d0 + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(d2 >> 8);
    uint32_t base = 0x00027E40u + (uint32_t)idx * 2u;
    int32_t s = (int16_t)rd16(m, base);
    int32_t c = (int16_t)rd16(m, base + 0x80u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)((int32_t)mag * c));
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)((int32_t)mag * s));
}

/* ROM 0x178DB6: add +0x06.w to +0x42, jitter +0x20.b by Obj_RandRange(+0x08.b), step 0x27E40. */
static void scr_178DB6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    uint16_t rng = Obj_RandRange(m, (uint16_t)rd8(m, a1 + 9u));
    uint8_t ang = (uint8_t)((uint8_t)rng + rd8(m, obj + 0x20u));
    wr8(m, obj + 0x20u, ang);
    wr16(m, obj + 0x42u, d1);
    int16_t mag = (int16_t)(uint16_t)(d1 + 0x20u) >> 6;
    uint32_t base = 0x00027E40u + (uint32_t)ang * 2u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178FCE: steer toward (+0x4C, +0x4E) while outside box |+0x42.b|. */
static void scr_178FCE(const f3mem_t *m, uint32_t obj)
{
    int16_t d2 = (int16_t)(int8_t)rd8(m, obj + 0x42u);
    if (d2 < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }
    int16_t dx = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t dy = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    int16_t ax = (dx < 0) ? (int16_t)(0u - (uint16_t)dx) : dx;
    int16_t ay = (dy < 0) ? (int16_t)(0u - (uint16_t)dy) : dy;
    if (ax <= d2 && ay <= d2) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    uint16_t d0 = Obj_Atan2(m, (uint16_t)dx, (uint16_t)dy);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t turn = rd16(m, a1 + 8u);
    int8_t diff = (int8_t)(uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    if (diff == 0)
        turn = 0u;
    else if (diff < 0)
        turn = (uint16_t)(0u - turn);

    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + turn));

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    uint32_t base = 0x00027E40u + (uint32_t)idx * 2u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x179074: aim toward (+0x4C, +0x4E) with random heading jitter. */
static void scr_179074(const f3mem_t *m, uint32_t obj)
{
    uint8_t d2b = rd8(m, obj + 0x42u);
    int16_t d2 = (int16_t)d2b;
    if (d2b & 0x80u) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }
    int16_t ex = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t ey = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    int16_t ax = (ex < 0) ? (int16_t)(0u - (uint16_t)ex) : ex;
    int16_t ay = (ey < 0) ? (int16_t)(0u - (uint16_t)ey) : ey;
    if (ax <= d2 && ay <= d2) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }
    uint16_t d0 = Obj_Atan2(m, (uint16_t)ex, (uint16_t)ey);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t jitter = rd16(m, a1 + 8u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    uint8_t ang = (uint8_t)(((uint8_t)d0 & (uint8_t)jitter)
                + (uint8_t)(int8_t)(int16_t)(jitter >> 8));
    wr8(m, obj + 0x20u, ang);
    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    uint32_t base = 0x00027E40u + (uint32_t)idx * 2u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178982 with all 1943CA..1947A8 script commands supported inline. */
static int script10_local(const f3mem_t *m, uint32_t obj)
{
    for (;;) {
        uint32_t a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);
        if (fn == 0u)
            goto loop_marker;
        switch (fn) {
        case 0x00178B02u: scr_178B02(m, obj); break;
        case 0x00178B0Eu: scr_178B0E(m, obj); break;
        case 0x00178B20u: Obj_Script10_SetHeading(m, obj); break;
        case 0x00178B7Au: scr_178B7A(m, obj); break;
        case 0x00178BB4u: scr_178BB4(m, obj); break;
        case 0x00178C06u: scr_178C06(m, obj); break;
        case 0x00178DB6u: scr_178DB6(m, obj); break;
        case 0x00178E4Au: Obj_Script10_SteerTo(m, obj); break;
        case 0x00178FCEu: scr_178FCE(m, obj); break;
        case 0x00179074u: scr_179074(m, obj); break;
        case 0x0017921Au: break;
        default:          return -1;
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
            if ((int32_t)rd32(m, a1) > 0)
                goto store;
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        continue;

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
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        return 0;
    }
}

/* ROM 0x17B02E: resolve score receiver from +0x35 (+0x36->+8 if 1, +0x36 if 2, else -1). */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        *a1_io = a1;
        return rd32(m, a1 + 0x08u);
    }
    if (k == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x17BA76: compute hit-spark point (out_x, out_y); returns < 0 if none. */
static int sub_17BA76(const f3mem_t *m, uint32_t obj, int16_t *out_x, int16_t *out_y)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 2u) {
        if ((uint16_t)(rd16(m, F3_A5 + 0x0878u) % 3u) != 0u)
            return -1;
        int16_t hw = (int16_t)((int16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)) >> 1);
        int16_t hh = (int16_t)((int16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u)) >> 1);
        int16_t ry = (int16_t)Obj_RandRange(m, (uint16_t)hh);
        int16_t rx = (int16_t)Obj_RandRange(m, (uint16_t)hw);
        *out_x = (int16_t)(rx + (int16_t)rd16(m, obj + 0x14u) + (int16_t)rd16(m, obj + 0x2Cu) + hw);
        *out_y = (int16_t)(ry + (int16_t)rd16(m, obj + 0x18u) + (int16_t)rd16(m, obj + 0x30u) + hh);
        return 0;
    }

    uint8_t st = rd8(m, a1 + 0x22u);
    if (st != 0u && st != 4u && (rd16(m, F3_A5 + 0x0878u) & 3u) != 0u)
        return -1;

    int16_t x0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu));
    int16_t y0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u));
    int16_t x1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Cu));
    int16_t y1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x30u));
    int16_t minx = (x1 > x0) ? x1 : x0;
    int16_t miny = (y1 > y0) ? y1 : y0;

    int16_t rx0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Eu));
    int16_t ry0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x32u));
    int16_t rx1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Eu));
    int16_t ry1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x32u));
    int16_t maxx = (rx1 < rx0) ? rx1 : rx0;
    int16_t maxy = (ry1 < ry0) ? ry1 : ry0;

    *out_x = (int16_t)(minx + (int16_t)((uint16_t)(maxx - minx) >> 1));
    *out_y = (int16_t)(miny + (int16_t)((uint16_t)(maxy - miny) >> 1));
    return 0;
}

/* ROM 0x17B9BA: spawn hit-spark 0x17A122 if +0x35 != 0 and Obj_RandRange(5) == 0. */
static void sub_17B9BA(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *d1_io)
{
    int16_t hx, hy;
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    uint16_t r = Obj_RandRange(m, 5u);
    *d0_io = setw(*d0_io, r);
    if (r != 0u)
        return;
    if (sub_17BA76(m, obj, &hx, &hy) < 0) {
        *d0_io = 0xFFFFFFFFu;
        return;
    }
    uint16_t dx = (uint16_t)(hx - (int16_t)rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(hy - (int16_t)rd16(m, obj + 0x54u));
    uint32_t arg = ((uint32_t)dx << 16) | dy;
    *d1_io = setw(*d1_io, (uint16_t)hy);
    *d0_io = Obj_Spawn(m, obj, 0x0017A122u, 5u, 2u, arg);
}

/* ROM 0x193DB6: push (+0x14, +0x18, +0x1C, +0x10) to 8-entry ring buffer at +0x68. */
static void sub_193DB6(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t idx = (uint16_t)((rd16(m, obj + 0x62u) + 1u) & 7u);
    wr16(m, obj + 0x62u, idx);
    uint32_t a1 = rd32(m, obj + 0x68u) + (uint32_t)idx * 0x0Au;
    wr16(m, a1, rd16(m, obj + 0x14u)); a1 += 2u;
    wr16(m, a1, rd16(m, obj + 0x18u)); a1 += 2u;
    wr16(m, a1, rd16(m, obj + 0x1Cu)); a1 += 2u;
    wr32(m, a1, rd32(m, obj + 0x10u)); a1 += 4u;
    *d0_io = (uint32_t)idx * 0x0Au;
    *a1_io = a1;
}

/* ROM 0x193E6C: stun/recoil player object on hit (+0x35 == 1). */
static void sub_193E6C(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    if (rd8(m, obj + 0x35u) != 1u)
        return;
    uint32_t a1 = rd32(m, obj + 0x36u);
    *a1_io = a1;
    uint8_t c = rd8(m, a1 + 0x22u);
    if (c == 0u || c == 4u || c == 5u || c == 6u)
        return;
    int32_t v = (int32_t)rd32(m, a1 + 0x48u);
    if (v >= 0)
        wr32(m, a1 + 0x48u, (uint32_t)(-v));
    uint16_t r = Obj_RandRange(m, 4u);
    *d0_io = setw(*d0_io, r);
    wr16(m, a1 + 0x4Cu, (uint16_t)(rd16(m, a1 + 0x4Cu) + r));
    wr8(m, a1 + 0x1Cu, (uint8_t)(rd8(m, a1 + 0x1Cu) | 0x01u));
    wr8(m, a1 + 0x1Du, 4u);
    wr32(m, a1 + 0x28u, 0u);
    Sound_PlayHooked(m, 0x0Fu);
}

/* ROM 0x193DE2: draw 4 ghost snapshots (indices 6, 4, 2, 0) from +0x68 ring. */
static void sub_193DE2(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    for (int16_t i = 7; i >= 0; i--) {
        if ((i & 1) != 0)
            continue;
        uint16_t idx = (uint16_t)((rd16(m, obj + 0x62u) - (uint16_t)i) & 7u);
        uint32_t a1 = rd32(m, obj + 0x68u) + (uint32_t)idx * 0x0Au;
        wr16(m, obj + 0x14u, rd16(m, a1)); a1 += 2u;
        wr16(m, obj + 0x18u, rd16(m, a1)); a1 += 2u;
        wr16(m, obj + 0x1Cu, rd16(m, a1)); a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;

        if (i == 0) {
            if (rd8(m, obj + 0x1Du) == 0x06u)
                wr8(m, obj + 0x1Du, 0x08u);
            else if (rd8(m, obj + 0x35u) != 0u)
                wr8(m, obj + 0x1Du, 0x06u);
            else
                wr8(m, obj + 0x1Du, 0u);
            wr8(m, obj + 0x35u, 0u);
        } else {
            wr8(m, obj + 0x1Du, 2u);
        }
        draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
    }
    *d0_io = setw(*d0_io, 0xFFFFu);
}

static void check_aura_spawn(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u &&
        (rd8(m, obj + 0x23u) & 0x80u) == 0u) {
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x80u));
        *d0_io = Obj_Spawn(m, obj, 0x0017ABDCu, 3u, 2u, 0x00000400u);
    }
}

int Fn_193F30(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];
    uint32_t a3 = rd32(m, obj + 0x0Cu);                  /* 0x193F30 */

    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {           /* 0x193F34 -> 0x1943AE */
        Obj_Free(m, rd32(m, obj + 0x58u));
        Obj_Free(m, obj);
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x193F3E -> 0x19431C */
        d0 = sub_17B02E(m, obj, &a1);
        if ((int32_t)d0 >= 0) {
            a1 = d0;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u);
        }
        wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);
        uint16_t id = rd8(m, obj + 0x22u);
        bf_set_mem(m, F3_A5 + 0x4B96u, id);
        d0 = Obj_Spawn(m, obj, 0x00179DEEu, 3u, 2u, 0u);
        Sound_PlayHooked(m, 0x16u);
        if ((int32_t)rd32(m, obj + 0x44u) <= 0) {
            wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x20u));
            wr16(m, F3_A5 + 0x4B20u, rd16(m, obj + 0x34u));
            wr32(m, F3_A5 + 0x4B1Cu, rd32(m, obj + 0x36u));
        }
        Obj_Free(m, rd32(m, obj + 0x58u));
        Obj_Free(m, obj);
        r->d[0] = d0;
        r->a[1] = a1;
        return 0;
    }

    {
        uint16_t id = rd8(m, obj + 0x22u);               /* 0x193F46 */
        d0 = id;
        if (bf_test_mem(m, F3_A5 + 0x4B3Au, id)) {       /* 0x193F4C -> 0x194008 */
            if ((int32_t)rd32(m, obj + 0x44u) <= 0) {
                uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);
                wr32(m, obj + 0x4Cu, tgt);
                if (tgt == 0u)
                    goto idle_mode;
                a3 = rd32(m, obj + 0x0Cu);
                uint32_t a2 = rd32(m, a3 + 4u);
                a1 = rd32(m, a2 + 0x18u);
                uint16_t idx = rd16(m, a1);
                a1 += 2u;
                wr16(m, obj + 0x40u, idx);
                wr32(m, obj + 0x3Au, a1);
                if (anim_init_local(m, obj, idx) < 0)
                    return -1;
                wr16(m, obj + 0x20u, rd16(m, a2 + 0x1Cu));
                wr8(m, obj + 0x34u, 0x14u);
                wr16(m, obj + 0x64u, 0x0100u);
                a1 = rd32(m, F3_A5 + 0x4B98u);
                a1 = rd32(m, a1 + (uint32_t)rd8(m, obj + 0x22u) * 4u);
                wr32(m, obj + 0x44u, a1);
                wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                wr16(m, obj + 0x4Au, 0u);
                Sound_PlayHooked(m, 0x3Au);
                d1 = 0u;
                for (int16_t d2w = 3; d2w >= 0; d2w--) {
                    uint16_t r0 = Obj_RandRange(m, 0x10u);
                    d1 = (d1 & 0xFFFFFF00u) | (uint8_t)((uint8_t)r0 + 0x80u);
                    d0 = Obj_Spawn(m, obj, 0x0017A1DEu, 5u, 2u, d1);
                }
            }

            {
                uint16_t tmp = rd16(m, obj + 0x20u);     /* 0x1940B0 */
                wr16(m, obj + 0x20u, rd16(m, obj + 0x66u));
                wr16(m, obj + 0x66u, tmp);
                if (rd8(m, obj + 0x35u) != 0u) {
                    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + 5u));
                    uint16_t r0 = Obj_RandRange(m, 5u);
                    d0 = setw(d0, r0);
                    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + r0));
                } else {
                    if (script10_local(m, obj) < 0)
                        return -1;
                }
                tmp = rd16(m, obj + 0x20u);              /* 0x1940E2 */
                wr16(m, obj + 0x20u, rd16(m, obj + 0x66u));
                wr16(m, obj + 0x66u, tmp);
            }

            wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));  /* 0x1940F0 */
            wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));  /* 0x1940F6 */
            check_aura_spawn(m, obj, &d0);               /* 0x1940FC */

            if ((int16_t)rd16(m, obj + 0x64u) < 0x0800)
                wr16(m, obj + 0x64u, (uint16_t)(rd16(m, obj + 0x64u) + 0x0020u));
            else
                wr16(m, obj + 0x64u, 0x0800u);

            d0 = setw(d0, rd16(m, obj + 0x64u));
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)d0));

            if (rd16(m, F3_A5 + 0x4B38u) != 0u && (rd16(m, F3_A5 + 0x0878u) & 0x001Fu) == 0u)
                Sound_PlayHooked(m, 0x2Bu);
            goto draw_step;
        }
    }

idle_mode:
    if ((int32_t)rd32(m, obj + 0x44u) >= 0) {            /* 0x194186 */
        if ((int32_t)rd32(m, obj + 0x44u) < 0x00194776) {
            a1 = 0x0019476Cu;
            wr32(m, obj + 0x44u, a1);
            wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
            wr16(m, obj + 0x4Au, 0u);
        }
        uint32_t parent = rd32(m, obj + 0x08u);          /* 0x1941AE */
        uint16_t tx = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t ty = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
        wr32(m, obj + 0x4Cu, ((uint32_t)tx << 16) | ty);

        uint16_t tmp = rd16(m, obj + 0x20u);             /* 0x1941CA */
        wr16(m, obj + 0x20u, rd16(m, obj + 0x66u));
        wr16(m, obj + 0x66u, tmp);
        if (script10_local(m, obj) < 0)                  /* 0x1941D8 */
            return -1;
        tmp = rd16(m, obj + 0x20u);                      /* 0x1941DE */
        wr16(m, obj + 0x20u, rd16(m, obj + 0x66u));
        wr16(m, obj + 0x66u, tmp);

        wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
        wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
        check_aura_spawn(m, obj, &d0);                   /* 0x1941F8 */

        d0 = setw(d0, rd16(m, obj + 0x64u));
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)d0));
    } else {                                             /* 0x19423C */
        if (rd16(m, obj + 0x40u) != 0x000Du) {
            int snap = 0;
            uint32_t a2 = 0u;
            if ((int16_t)rd16(m, obj + 0x64u) <= 0x0400) {
                a2 = rd32(m, a3 + 4u);
                uint8_t d0b = (uint8_t)((rd8(m, a2 + 0x1Cu) - 0x10u) & 0xFCu);
                uint8_t d1b = (uint8_t)(rd8(m, obj + 0x20u) & 0xFCu);
                d0 = (d0 & 0xFFFFFF00u) | d0b;
                d1 = (d1 & 0xFFFFFF00u) | d1b;
                if (d0b == d1b)
                    snap = 1;
            }
            if (!snap) {                                 /* 0x19426E */
                if ((int16_t)rd16(m, obj + 0x64u) > 0x0400)
                    wr16(m, obj + 0x64u, (uint16_t)(rd16(m, obj + 0x64u) - 0x0080u));
                else
                    wr16(m, obj + 0x64u, 0x0400u);
                d0 = setw(d0, rd16(m, obj + 0x64u));
                wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)d0));
            } else {                                     /* 0x194294 */
                a1 = rd32(m, a2 + 0x12u);
                uint16_t idx = rd16(m, a1);
                a1 += 2u;
                wr16(m, obj + 0x40u, idx);
                wr32(m, obj + 0x3Au, a1);
                if (anim_init_local(m, obj, idx) < 0)
                    return -1;
                wr8(m, obj + 0x34u, rd8(m, a2 + 0x0Cu));
                Sound_PlayHooked(m, 0x3Au);
                d1 = 0u;
                for (int16_t d2w = 3; d2w >= 0; d2w--) {
                    uint16_t r0 = Obj_RandRange(m, 0x10u);
                    d1 = (d1 & 0xFFFFFF00u) | (uint8_t)r0;
                    d0 = Obj_Spawn(m, obj, 0x0017A1DEu, 5u, 2u, d1);
                }
            }
        }
        uint32_t parent = rd32(m, obj + 0x08u);          /* 0x1942F4 */
        uint16_t px = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t py = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, px);
        wr16(m, obj + 0x54u, py);
        wr16(m, obj + 0x14u, px);
        wr16(m, obj + 0x18u, py);
    }

draw_step:
    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {       /* 0x193F5A */
        uint32_t p = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x08FEu, p + 4u);
        p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        a1 = p + 4u;
        wr32(m, F3_A5 + 0x0906u, a1);
        if ((rd8(m, obj + 0x34u) & 0x02u) != 0u)
            sub_17B9BA(m, obj, &d0, &d1);
        wr8(m, obj + 0x35u, 0u);
    }

    {
        uint16_t idx = rd16(m, obj + 0x40u);             /* 0x193F8C */
        if (anim_tick_local(m, obj, idx) < 0)
            return -1;
    }

    {
        uint32_t spr = rd32(m, obj + 0x10u);             /* 0x193F9C */
        uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);
        uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w));
        sub_193DB6(m, obj, &d0, &a1);                    /* 0x193FB0 */
    }

    if (rd16(m, obj + 0x40u) == 0x000Du) {               /* 0x193FB4 */
        if (rd8(m, obj + 0x1Du) == 0x06u)
            wr8(m, obj + 0x1Du, 0x08u);
        else if (rd8(m, obj + 0x35u) != 0u)
            wr8(m, obj + 0x1Du, 0x06u);
        else
            wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
        wr8(m, obj + 0x35u, 0u);
        draw_sprite_regs(m, obj, &d0, &d1, &a1);
    } else {
        sub_193E6C(m, obj, &d0, &a1);                    /* 0x193FBE */
        sub_193DE2(m, obj, &d0, &d1, &a1);               /* 0x193FC2 */
    }

    wr32(m, obj, 0x00193F30u);                           /* 0x194004 -> 0x193F28 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
