#include "fn_auto.h"
/* ROM 0x0E40DE: scroll, animate and aim the enemy, and handle bomb or hit deaths. */
#include "fn_call_frames.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0D0690 with D0/A1 register tracking: one 12-byte script step when the
 * +0x4C timer runs out. */
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
            *d0_io = set_word(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = set_word(*d0_io, pal);
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
    *d0_io = set_word(*d0_io, pal);
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

/* ROM 0x0D0BBC: turn +0x08 toward +0x42 by the step in (A1+4). */
static void turn_toward_target(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], rec = r->a[1];
    uint16_t angle = rd16(m, obj + 0x42u);
    r->d[1] = set_word(r->d[1], angle);
    uint16_t diff = (uint16_t)(angle - rd16(m, obj + 8u));
    if ((int16_t)diff < 0)
        diff = (uint16_t)(diff + 0x168u);
    int backwards = diff > 0xB4u;
    if (backwards)
        diff = (uint16_t)(0x168u - diff);
    r->d[0] = set_word(r->d[0], diff);
    if (diff <= rd16(m, rec + 4u)) {
        wr16(m, obj + 8u, angle);
        return;
    }

    r->d[0] = rd32(m, rec + 4u);
    uint32_t heading = rd32(m, obj + 8u);
    heading = backwards ? heading - r->d[0] : heading + r->d[0];
    wr32(m, obj + 8u, heading);
    if (backwards && (int32_t)heading < 0)
        wr16(m, obj + 8u, (uint16_t)((heading >> 16) + 0x168u));
    if (!backwards && (heading >> 16) >= 0x168u)
        wr16(m, obj + 8u, (uint16_t)((heading >> 16) - 0x168u));
}

/* ROM 0x0D25E4 */
static void sub_0D25E4(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d4w = 0u;

    while (d1w >= 0x0032u) {
        d4w = (uint16_t)(d4w + 1u);
        d1w = (uint16_t)(d1w - 0x0032u);
    }

    uint16_t d2w = d1w;
    int32_t idx = (int32_t)(int16_t)d0w;
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2044u + (uint32_t)idx * 4u));
    uint32_t d0 = rd32(m, (uint32_t)(0x000D1EDCu + (uint32_t)idx * 4u));

    /* Jump table 0x0D4F26 ([0..50] at 0x0D4FF2..0x0D538A): exact (D0*k, D1*k) mod 2^32 */
    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    uint32_t d3 = d0;
    d2w = d4w;
    uint32_t d4 = d1;

    d1 = rd32(m, (uint32_t)(0x000D27A4u + (uint32_t)idx * 4u));
    d0 = rd32(m, (uint32_t)(0x000D263Cu + (uint32_t)idx * 4u));

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k returns the
         * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)
         * mod 2^32 is exact for every k the table has. */
    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    *out_d0 = (uint32_t)(d0 + d3);
    *out_d1 = (uint32_t)(d1 + d4);
}

/* ROM 0x0D346C */
static uint32_t sub_0D346C(const f3mem_t *m, uint16_t d0w, uint16_t d1w)
{
    uint32_t a0 = rd32(m, (uint32_t)(0x000D348Eu + (uint32_t)(int32_t)(int16_t)d1w * 4u));
    if ((int16_t)d0w < 0) {
        d0w = (uint16_t)(0u - d0w);
        uint32_t d0 = rd32(m, (uint32_t)(a0 + (uint32_t)(int32_t)(int16_t)d0w * 4u));
        return (uint32_t)(0u - d0);
    }
    return rd32(m, (uint32_t)(a0 + (uint32_t)(int32_t)(int16_t)d0w * 4u));
}

/* ROM 0x0CF1E4 */
static void run_script(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    for (;;) {
        uint32_t a1 = rd32(m, obj + 0x44u);
        uint16_t op = rd16(m, a1);
        a1 += 2u;
        r->d[0] = set_word(r->d[0], op);
        wr8(m, obj + 0x20u, (uint8_t)op);

        switch (op) {
        case 0u: /* ROM 0x0CF278 */
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & ~0x80u));
            break;

        case 1u: /* ROM 0x0CF280 */
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x80u);
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x04u);
            break;

        case 2u: /* ROM 0x0CF28E */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x40u);
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x04u);
            break;

        case 3u: /* ROM 0x0CF2A2 */
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) & ~0x40u);
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x04u);
            break;

        case 4u: /* ROM 0x0CF2B6 */
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x20u);
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x04u);
            break;

        case 5u: /* ROM 0x0CF2C4 */
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) & ~0x20u);
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x04u);
            break;

        case 6u: /* ROM 0x0CF2D2 */
            wr32(m, obj + 0x58u, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 7u: /* ROM 0x0CF2DE */
            wr32(m, obj + 0x5Cu, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 8u: /* ROM 0x0CF2EA */
            wr32(m, obj + 0x60u, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 9u: { /* ROM 0x0CF2F6 */
            uint16_t d0w = (uint16_t)rd8(m, r->a[5] + 0x087Fu);
            d0w = (uint16_t)(d0w + 1u);
            d0w = (uint16_t)(d0w << 2);
            d0w = (uint16_t)(d0w + 0x0C00u);
            uint32_t d0 = ((uint32_t)d0w * (uint32_t)rd16(m, a1)) << 4;
            r->d[0] = d0;
            wr32(m, obj + 0x60u, d0);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 10u: /* ROM 0x0CF314 */
            wr32(m, obj + 0x64u, rd32(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 11u: { /* ROM 0x0CF320 */
            uint32_t d0 = (uint32_t)rd8(m, r->a[5] + 0x087Fu);
            d0 = ((d0 + 1u) >> 2) + 0xC0u;
            uint32_t d1 = rd32(m, a1);
            if ((int32_t)d1 < 0) {
                d1 = (uint32_t)(0u - d1);
                d0 = (uint32_t)(d0 * d1);
                d0 >>= 8;
                d0 = (uint32_t)(0u - d0);
            } else {
                d0 = (uint32_t)(d0 * d1);
                d0 >>= 8;
            }
            r->d[0] = d0;
            r->d[1] = d1;
            wr32(m, obj + 0x64u, d0);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 12u: { /* ROM 0x0CF35E */
            uint32_t d0_out, d1_out;
            sub_0D25E4(m, rd16(m, obj + 0x58u), rd16(m, a1), &d0_out, &d1_out);
            r->d[0] = d0_out;
            r->d[1] = d1_out;
            wr32(m, obj + 0x50u, d0_out);
            wr32(m, obj + 0x54u, d1_out);
            break;
        }

        case 13u: { /* ROM 0x0CF374 */
            uint16_t d0w = rd16(m, a1);
            a1 += 2u;
            uint16_t d1w = rd16(m, obj + 0x60u);
            r->d[1] = set_word(r->d[1], d1w);
            r->d[0] = sub_0D346C(m, d0w, d1w);
            wr32(m, obj + 0x5Cu, r->d[0]);
            wr32(m, obj + 0x50u, 0u);
            wr16(m, obj + 0x50u, rd16(m, a1));
            break;
        }

        case 14u: /* ROM 0x0CF38E */
        case 15u: /* ROM 0x0CF38E */
        case 20u: /* ROM 0x0CF38E */
            wr16(m, obj + 0x50u, rd16(m, a1));
            break;

        case 16u: /* ROM 0x0CF394 */
            r->d[0] = set_word(r->d[0], rd16(m, a1));
            wr8(m, obj + 0x21u, (uint8_t)r->d[0]);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 17u: { /* ROM 0x0CF3A2 */
            uint8_t cnt = (uint8_t)(rd8(m, obj + 0x21u) - 1u);
            wr8(m, obj + 0x21u, cnt);
            if (cnt != 0u) {
                r->d[0] = rd32(m, a1);
                wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - r->d[0]);
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 18u: /* ROM 0x0CF3B8 */
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x10u);
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x04u);
            break;

        case 19u: /* ROM 0x0CF3C6 */
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) | 0x08u);
            wr8(m, obj + 0x07u, rd8(m, obj + 0x07u) & ~0x80u);
            break;

        case 21u: /* ROM 0x0CF3D4 */
            wr16(m, obj + 0x1Eu, rd16(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 22u: { /* ROM 0x0CF3E0 */
            uint8_t cur = rd8(m, obj + 0x1Eu);
            uint8_t delta = rd8(m, a1);
            r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)(cur - delta);
            if (cur < delta) {
                wr16(m, obj + 0x1Eu, 0u);
            } else {
                uint8_t nv = (uint8_t)(cur - delta);
                wr8(m, obj + 0x1Eu, nv);
                wr8(m, obj + 0x1Fu, nv);
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 23u: { /* ROM 0x0CF406 */
            unsigned sum = (unsigned)rd8(m, obj + 0x1Eu) + (unsigned)rd8(m, a1);
            r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)sum;
            if (sum > 0xFFu) {
                wr16(m, obj + 0x1Eu, 0xFFFFu);
            } else {
                wr8(m, obj + 0x1Eu, (uint8_t)sum);
                wr8(m, obj + 0x1Fu, (uint8_t)sum);
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 24u: { /* ROM 0x0CF42E */
            uint8_t cur = rd8(m, obj + 0x1Eu);
            uint8_t delta = rd8(m, a1);
            r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)(cur - delta);
            if (cur < delta) {
                wr16(m, obj + 0x1Eu, 0u);
            } else {
                uint8_t nv = (uint8_t)(cur - delta);
                wr8(m, obj + 0x1Eu, nv);
                wr8(m, obj + 0x1Fu, nv);
                r->d[0] = rd16(m, a1 + 2u);
                wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - r->d[0]);
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 25u: { /* ROM 0x0CF460 */
            unsigned sum = (unsigned)rd8(m, obj + 0x1Eu) + (unsigned)rd8(m, a1);
            r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)sum;
            if (sum > 0xFFu) {
                wr16(m, obj + 0x1Eu, 0xFFFFu);
            } else {
                wr8(m, obj + 0x1Eu, (uint8_t)sum);
                wr8(m, obj + 0x1Fu, (uint8_t)sum);
                r->d[0] = rd16(m, a1 + 2u);
                wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - r->d[0]);
            }
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;
        }

        case 26u: /* ROM 0x0CF494 */
            wr8(m, obj + 0x06u, rd8(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 27u: /* ROM 0x0CF4A0 */
            wr8(m, obj + 0x22u, rd8(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        case 28u: /* ROM 0x0CF4AC */
            wr8(m, obj + 0x23u, rd8(m, a1));
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x04u));
            break;

        default:
            return; /* The ROM table at 0x0CF204 contains opcodes 0–28. */
        }

        r->a[1] = a1;
        wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 6u);
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x04u));
        if ((f7 & 0x04u) == 0u)
            break;
    }
}

/* ROM 0x00649A with full D0 result. */
static uint32_t dist_full(const f3mem_t *m, uint16_t dx_in, uint16_t dy_in)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)dx_in * (int32_t)(int16_t)dx_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)dy_in * (int32_t)(int16_t)dy_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    return (d0 << 16) | (d0 >> 16);
}


/* 0x0D0E52 uses a full-register exchange in two quadrants. */
static void aim_angle(const f3mem_t *m, f3regs_t *r)
{
    uint16_t x = (uint16_t)r->d[0], y = (uint16_t)r->d[1];
    unsigned quadrant = 0;
    if ((int16_t)x < 0) {
        quadrant += 2u;
        x = (uint16_t)-x;
    }
    if ((int16_t)y < 0) {
        quadrant += 4u;
        y = (uint16_t)-y;
    }
    uint16_t magnitude = x > y ? x : y;
    unsigned shift = 0;
    while (magnitude >= 64u) {
        magnitude >>= 1;
        ++shift;
    }
    x >>= shift;
    y >>= shift;
    if (quadrant == 0u || quadrant == 6u) {
        uint32_t saved = r->d[0];
        r->d[0] = r->d[1];
        r->d[1] = saved;
        uint16_t temp = x;
        x = y;
        y = temp;
    }
    uint16_t index = (uint16_t)((y << 6) + x);
    uint16_t angle = (uint16_t)(rd8(m, 0x0D0EBCu + (int16_t)index) +
                                rd16(m, 0x0D0EB4u + quadrant));
    r->d[0] = set_word(r->d[0], angle == 0x168u ? 0u : angle);
    r->d[1] = set_word(r->d[1], index);
}

/* 0x0D07E4 and 0x0D08A0 inspect each live player with the same bounds. */
static void aim_at_players(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], rec = r->a[1];
    uint32_t muzzle = rd32(m, obj + 0x68u);
    r->a[2] = muzzle + 2u;
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, muzzle));
    r->d[0] = set_word(r->d[0], (uint16_t)x);
    if (x < 0x28 || x > 0x127)
        return;
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, muzzle + 2u));
    r->d[0] = set_word(r->d[0], (uint16_t)y);
    if (y < 0x10 || y > 0xDF)
        return;

    for (unsigned i = 0; i < 2u; ++i) {
        unsigned dx_reg = 2u + 3u * i, dy_reg = dx_reg + 1u, angle_reg = dx_reg + 2u;
        uint32_t player = 0x41B190u + 0x8Au * i;
        r->a[2] = player;
        r->d[angle_reg] = UINT32_MAX;
        if ((int16_t)rd16(m, player + 0x4Cu) < 0)
            continue;
        r->a[4] = muzzle + 2u;
        int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, player + 0x18u)
                               + rd16(m, r->a[4]) - 0xFu);
        r->a[3] = set_word(r->d[dy_reg], (uint16_t)dy);
        uint16_t abs_y = (uint16_t)(dy < 0 ? -dy : dy);
        r->d[dy_reg] = set_word(r->d[dy_reg], abs_y);
        uint16_t reach = rd16(m, rec + 0xAu);
        if (abs_y > reach)
            continue;
        int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, player + 0x14u)
                               + rd16(m, muzzle) - 0x17u);
        r->a[4] = set_word(r->d[dx_reg], (uint16_t)dx);
        uint16_t abs_x = (uint16_t)(dx < 0 ? -dx : dx);
        r->d[dx_reg] = set_word(r->d[dx_reg], abs_x);
        if (abs_x > reach)
            continue;
        uint16_t near = rd16(m, rec + 8u);
        if (dx >= 0)
            r->d[0] = set_word(r->d[0], (uint16_t)(near - 0x1Eu));
        if (abs_y < near && (dx < 0 ? abs_x < near : dx < (int16_t)(near - 0x1Eu)))
            continue;

        r->d[0] = set_word(r->d[0], (uint16_t)dx);
        r->d[1] = set_word(r->d[1], (uint16_t)dy);
        aim_angle(m, r);
        uint16_t low = rd16(m, rec + 0xCu), high = rd16(m, rec + 0xEu);
        r->d[1] = set_word(r->d[1], low);
        uint16_t angle = (uint16_t)r->d[0];
        if (low > high ? angle >= low || angle <= high : angle >= low && angle <= high)
            r->d[angle_reg] = set_word(r->d[angle_reg], angle);
    }

    uint16_t angle = (uint16_t)r->d[7];
    r->d[0] = set_word(r->d[0], angle);
    if ((int16_t)r->d[4] >= 0) {
        if ((int16_t)angle >= 0) {
            for (unsigned i = 2u; i <= 5u; i += 3u) {
                if ((uint16_t)r->d[i + 1u] > (uint16_t)r->d[i])
                    r->d[i] = set_word(r->d[i], (uint16_t)r->d[i + 1u]);
            }
        }
        if ((int16_t)angle < 0 || (uint16_t)r->d[5] >= (uint16_t)r->d[2]) {
            angle = (uint16_t)r->d[4];
            r->a[2] = 0x41B190u;
        }
    }
    if ((int16_t)angle < 0)
        return;
    wr16(m, obj + 0x42u, angle);
    uint16_t difficulty = rd16(m, 0x0D0D02u + 2u * (rd8(m, r->a[5] + 0x87Fu) >> 3));
    uint16_t threshold = (uint16_t)(rd16(m, rec + 0x10u) + difficulty);
    r->d[1] = set_word(r->d[1], threshold);
    uint16_t roll = Obj_Next4Way(m);
    r->d[0] = set_word(r->d[0], roll);
    if ((int16_t)threshold > (int16_t)roll) {
        uint16_t timer = rd16(m, rec + 0x12u);
        r->d[0] = set_word(r->d[0], timer);
        wr8(m, obj + 0x23u, (uint8_t)timer);
    }
}

/* 0x0D005A selects a long pointer using the signed word at 0x0D00B4. */
static void install_direction(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t offset = (int16_t)rd16(m, 0x0D00B4u + 2 * (int16_t)rd16(m, obj + 8u));
    r->d[0] = set_word(r->d[0], (uint16_t)offset);
    uint32_t entry = rd32(m, r->a[1] + offset);
    uint32_t key = rd32(m, obj + 0xCu) + 0x28u;
    r->a[1] = entry;
    r->a[2] = key - 0x28u;
    if (entry == rd32(m, key))
        return;
    wr32(m, obj + 0x48u, entry);
    wr32(m, key, entry);
    wr16(m, obj + 0x4Cu, rd16(m, entry));
    r->d[0] = set_word(r->d[0], rd16(m, entry + 2u));
    wr8(m, obj + 0x1Cu, (uint8_t)r->d[0]);
    wr32(m, obj + 0x10u, rd32(m, entry + 4u));
    wr32(m, obj + 0x68u, rd32(m, entry + 8u));
    r->a[1] = entry + 8u;
}

/* 0x0CF4B8: the shared motion script writes object memory. Track its outputs. */
static void move_script(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint8_t op = rd8(m, obj + 0x20u);
    r->d[0] = set_word(r->d[0], op);
    if (op == 13u)
        r->d[2] = rd32(m, obj + 0x5Cu);
    if (op == 14u)
        r->d[0] = rd32(m, obj + 0x64u);
    if (op == 15u) {
        r->a[2] = 0x41B190u;
        r->a[3] = 0x41B21Au;
        int first = (int16_t)rd16(m, r->a[2] + 0x4Cu) >= 0;
        int second = (int16_t)rd16(m, r->a[3] + 0x4Cu) >= 0;
        if (first && second) {
            uint16_t distance[2];
            for (unsigned i = 0; i < 2u; ++i) {
                uint32_t player = r->a[2u + i];
                int16_t dx = (int16_t)(rd16(m, obj + 0x3Au) - rd16(m, player + 0x14u)
                                       + rd16(m, r->a[1]) - 0x17u);
                int16_t dy = (int16_t)(rd16(m, obj + 0x3Eu) - rd16(m, player + 0x18u)
                                       + rd16(m, r->a[1] + 2u) - 0xFu);
                uint16_t x = (uint16_t)(dx < 0 ? -dx : dx);
                uint16_t y = (uint16_t)(dy < 0 ? -dy : dy);
                distance[i] = x > y ? x : y;
                if (i == 1u)
                    r->d[2] = set_word(r->d[2], y);
            }
            if (distance[1] < distance[0])
                r->a[2] = r->a[3];
        } else if (!first && second) {
            r->a[2] = r->a[3];
        }
    }
    (void)ObjScript_CF4B8(m, obj, r->a[1]);
    if (op >= 12u && op <= 15u)
        sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &r->d[0], &r->d[1]);
}
/* ROM 0x001282: draw through the shared renderer and carry its scratch registers. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t def = rd32(m, obj + 0x10u);
    r->a[2] = def;
    if (rd32(m, def) == 0x00010001u) {
        uint32_t attr = rd16(m, def + 6u);
        uint16_t flip = rd16(m, obj + 0x1Cu) & 0xFF00u;
        attr ^= flip;
        uint8_t colour = rd8(m, obj + 0x1Du);
        if (colour)
            attr = (attr & 0xFF00u) | colour;
        r->d[0] = ((uint32_t)(rd16(m, obj + 0x14u) & 0xFFFu) << 16) | attr;
        r->d[1] = set_word(r->d[1], flip);
    } else {
        uint16_t cols = rd16(m, def), rows = rd16(m, def + 2u);
        uint32_t row = def + 4u;
        uint8_t flip = rd8(m, obj + 0x1Cu);
        uint8_t colour = rd8(m, obj + 0x1Du);
        int16_t dx = (flip & 2u) ? -4 : 4;
        int16_t step = (int16_t)(cols * 4u);
        r->d[2] = flip;
        if (flip & 1u) {
            step = (int16_t)-step;
            r->d[2] = (uint32_t)((int16_t)(rows - 1u) * step);
            row -= (int16_t)r->d[2];
        }
        if (flip & 2u)
            row += 4u * (uint32_t)(int32_t)(int16_t)(cols - 1u);
        r->d[6] = set_word(r->d[6], (uint16_t)dx);
        r->d[7] = set_word(r->d[7], (uint16_t)step);
        r->d[4] = set_word(r->d[4], rd16(m, obj + 0x1Cu) & 0xFF00u);
        uint16_t dy = (uint16_t)(rd8(m, obj + 0x1Eu) - 0x100u);
        r->d[5] = ((uint32_t)(uint16_t)(rd8(m, obj + 0x1Fu) - 0x101u) << 16) | dy;
        r->d[2] = set_word(r->d[2], rd16(m, obj + 0x14u));
        r->d[3] = r->d[2] & 0xFFFu;
        for (uint32_t y = 0; y < (rows ? rows : 65536u); ++y) {
            uint32_t cell = row;
            r->a[3] = row;
            r->d[2] = set_word(r->d[2], (uint16_t)(rd16(m, obj + 0x18u) << 4));
            r->d[4] &= ~0x80003000u;
            for (uint32_t x = 0; x < (cols ? cols : 65536u); ++x) {
                if (rd16(m, cell)) {
                    r->d[3] = set_word(r->d[3], (uint16_t)(rd16(m, cell + 2u) ^ r->d[4]));
                    if (colour)
                        r->d[3] = (r->d[3] & 0xFFFFFF00u) | colour;
                    if (!(r->d[4] & 0x1000u)) {
                        r->d[4] = (r->d[4] & ~0x8000u) | 0x80007000u;
                        r->d[3] = (r->d[2] >> 4) & 0xFFFu;
                    }
                } else {
                    r->d[4] &= ~0x3000u;
                }
                cell += dx;
                r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] - dy));
            }
            row += step;
            if (!(r->d[4] & 0x80000000u))
                r->d[4] |= 0x4000u;
            r->d[4] |= 0x8000u;
        }
        r->a[2] = row;
        r->d[0] = 0xFFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0xFFFFu;
    }
    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, r->a[5] - 0x0CA0u);
}

/* Bomb entry 0x0E4288 and 0x0CEF0E. The pause gate bypasses 0x0CBB2C. */
static void bomb_death(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->d[0] = 0x12Cu;
    push_long(m, r, 0x0E4294u);
    r->a[1] = rd32(m, r->a[5] + 0x1D50u);
    Obj_RelayTimer(m, r->d[0]);
    r->a[7] += 4u;
    wr8(m, obj + 6u, 0u);
    if (rd8(m, r->a[5] - 0x144Bu) & 0x20u) {
        Obj_StartDying(m, obj);
        return;
    }

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (!(rd8(m, r->a[5] + 0x1D58u) & 2u) || x < -10 || x > 0x11C) {
        wr32(m, obj, 0x0CEF44u);
        return;
    }

    /* 0x0CEF36 tails into the shared explosion at 0x0CBB2C. */
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr8(m, obj + 0x32u, 0u);
    wr8(m, obj + 0x34u, 0xFFu);
    uint32_t def = rd32(m, obj + 0x10u);
    r->a[1] = def + 4u;
    int16_t prod_y = (int16_t)(((int)rd8(m, obj + 0x1Eu) - 0x100) * (int16_t)rd16(m, def));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (prod_y >> 5)));
    int32_t prod_x = ((int)rd8(m, obj + 0x1Fu) - 0x100) * (int16_t)rd16(m, def + 2u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - ((int16_t)prod_x >> 5)));
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0xA0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x74u);
    r->d[0] = set_word((uint32_t)prod_x, dx);
    r->d[1] = set_word(r->d[1], dy);
    uint32_t distance = dist_full(m, dx, dy);
    uint16_t band = (uint16_t)distance << 2;
    uint16_t threshold = rd16(m, r->a[5] + 0x1D5Cu);
    r->d[0] = set_word(distance, (uint16_t)(band - threshold));
    if ((int16_t)band < (int16_t)threshold) {
        wr32(m, obj, 0x0CBC88u); /* 0x0CBB98 -> 0x0CBC80 */
        return;
    }

    /* 0x0CBB9C–0x0CBC66 prepares the first explosion sprite. */
    uint16_t radius = (uint16_t)((uint16_t)(band - threshold) << 6);
    wr16(m, obj + 0x2Cu, radius);
    uint16_t angle = (uint16_t)(Obj_Atan2(m, dx, dy) << 8);
    uint32_t angle_high = ((int16_t)dx < 0) != ((int16_t)dy < 0) ? r->d[1] : r->d[0];
    r->d[0] = set_word(angle_high, angle);
    wr16(m, obj + 0x2Eu, angle);
    wr16(m, obj + 0x30u, 0x100u);
    if (!(rd8(m, r->a[5] + 0x1D58u) & 2u)) {
        wr32(m, obj, 0x0CBC88u); /* 0x0CBBCA -> 0x0CBC80 */
        return;
    }
    angle = (uint16_t)(angle + 0x200u);
    wr16(m, obj + 0x2Eu, angle);
    radius = (uint16_t)(radius - (radius >> 5));
    wr16(m, obj + 0x2Cu, radius);
    int16_t scaled_radius = (int16_t)((radius >> 6) + threshold);
    uint32_t index = 2u * (angle >> 8);
    r->d[1] = (uint32_t)(scaled_radius * (int16_t)rd16(m, 0x027EC0u + index)) + 0xA00000u;
    wr32(m, obj + 0x14u, r->d[1]);
    r->d[2] = (uint32_t)(scaled_radius * (int16_t)rd16(m, 0x027E40u + index)) + 0x740000u;
    wr32(m, obj + 0x18u, r->d[2]);
    wr8(m, obj + 0x1Fu, 0u);
    wr8(m, obj + 0x1Eu, 0u);
    r->d[0] = (uint32_t)(-0x100 * (int16_t)rd16(m, def));
    int16_t offset = (int16_t)(uint16_t)r->d[0] >> 5;
    r->d[0] = set_word(r->d[0], (uint16_t)offset);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + offset));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + offset));
    push_long(m, r, 0x0CBC70u);
    draw_sprite_regs(m, r);
    r->a[7] += 4u;
    wr32(m, obj, 0x0CBC78u);
}

int Fn_0E40DE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], a5 = r->a[5];
    if (rd8(m, obj + 6u) == 2u) {
        r->d[0] = rd32(m, a5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - r->d[0]);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    }
    r->d[0] = rd32(m, a5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - r->d[0]);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if ((rd8(m, a5 + 0x1D58u) & 2u) && x >= -10 && x <= 0x11C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        bomb_death(m, r);
        goto done;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        r->d[0] = 0x12Cu;
        push_long(m, r, 0x0E4226u);
        uint8_t score_target = rd8(m, obj + 0x35u);
        if (score_target == 1u)
            r->a[1] = rd32(m, rd32(m, obj + 0x36u) + 8u);
        if (score_target == 2u)
            r->a[1] = rd32(m, obj + 0x36u);
        Obj_0D0E04(m, obj, r->d[0]);
        r->a[7] += 4u;
        goto burst;
    }
    if (rd8(m, a5 - 0x144Bu) & 0x20u)
        goto burst;
    if (rd8(m, obj + 6u) == 2u)
        goto animate;

    uint8_t flags = rd8(m, obj + 7u);
    wr8(m, obj + 7u, flags & ~2u);
    if (flags & 2u) {
        push_long(m, r, 0x0E4154u);
        run_script(m, r);
        r->a[7] += 4u;
    }
    if (rd8(m, obj + 0x20u)) {
        r->a[1] = 0x0E3054u; /* 0x0E415E + (int16_t)0xEEF6 */
        push_long(m, r, 0x0E4166u);
        move_script(m, r);
        r->a[7] += 4u;
    }
    push_long(m, r, 0x0E416Cu);
    Obj_ArmOnScreen(m, obj);
    r->a[7] += 4u;
    push_long(m, r, 0x0E4172u);
    Obj_FlagOffscreen(m, obj);
    r->a[7] += 4u;
    if (rd8(m, obj + 7u) & 8u)
        goto die;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    if (rd8(m, obj + 0x23u) == 0u) {
        if (!(rd8(m, obj + 7u) & 0x20u))
            goto animate;
        r->a[1] = 0x0E3054u;
        push_long(m, r, 0x0E41A4u);
        aim_at_players(m, r);
        r->a[7] += 4u;
        push_long(m, r, 0x0E41AAu);
        turn_toward_target(m, r);
        r->a[7] += 4u;

        r->a[1] = 0x0E31BCu;
        push_long(m, r, 0x0E41B8u);
        install_direction(m, r);
        r->a[7] += 4u;
        if (rd8(m, obj + 6u) == 3u)
            wr8(m, obj + 0x23u, 0u);
        if (rd8(m, obj + 0x23u) == 0u)
            goto animate;
    }
    uint8_t timer = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, timer);
    if (!timer) {
        call_spawn(m, r, 0x0E41EEu, 0x0E35BEu, 9u, 2u, 0u);
        if ((int32_t)r->d[0] >= 0)
            wr8(m, obj + 6u, 2u);
    }

animate: /* 0x0E41FE */
    push_long(m, r, 0x0E4204u);
    step_script_regs(m, obj, &r->d[0], &r->a[1]);
    r->a[7] += 4u;
    push_long(m, r, 0x0E420Au);
    Obj_BlinkColour(m, obj);
    r->a[7] += 4u;
    if (rd8(m, obj + 0x1Eu) == 0u) {
        push_long(m, r, 0x0E4218u);
        add_coll_regs(m, obj, &r->a[1]);
        r->a[7] += 4u;
    }
    goto done;

burst: /* 0x0E4226 */
    call_spawn(m, r, 0x0E423Eu, 0x0DC6D0u, 3u, 2u, 0u);
    if ((int32_t)r->d[0] >= 0) {
        uint32_t child = r->d[0];
        r->a[1] = child;
        wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
        wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, child + 0x58u, 0xB4u);
        wr16(m, child + 0x60u, rd16(m, obj + 0x60u));
        wr16(m, child + 0x10u, rd16(m, obj + 0x1Eu));
        r->d[1] = rd32(m, 0x0D4A5Au + 4u * rd8(m, obj + 0x1Eu));
        uint32_t product = r->d[1] << 4;
        r->d[1] = 0x10u;
        r->d[0] = (product << 16) | (product >> 16);
        wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + r->d[0]));
        wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + r->d[0]));
    }

die: /* 0x0E427E -> 0x0CEE74 */
    wr8(m, obj + 6u, 0u);
    Obj_StartDying(m, obj);
done:
    r->a[7] += 4u;
    return 0;
}
