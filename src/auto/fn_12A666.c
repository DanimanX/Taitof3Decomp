/* ROM 0x12A666: boss sub-unit/turret controller tick (A0 = object).
 * Propagates parent (+0x08)->+0x59 to self and frees self (0x0105EC) if
 * nonzero. Transfers damage (+0x24) into shared boss HP pool A5+0x30DA via
 * 0x11AE62, runs phase setup table 0x12ADBC[A5+0x30F2] (modes 0..8; 9..14 are
 * bare rts no-ops), eases +0x20 toward A5+0x3280 and A5+0x327E toward +0x42.
 * When A5+0x30DA < 0x1F4, triggers explosion fx (0x11C096), locks +0x20 =
 * 0x7800, spawns 0x13E4EE and plays sounds 0x16/0x14 on first break. Updates
 * sprite/hitbox via 0x12B41E and 0x11B06E, positions at (A5+0x30D2,
 * A5+0x30D6 - 0x40), fires triples of 0x0CD124, quads of 0x11B5E6, 5x
 * 0x12B934, and 0x12BDB2 based on A5+0x30EE flags, then re-arms 0x12A666. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x00649A with full 32-bit D0 result (preserves D1 and A0). */
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

/* ROM 0x0CC33A: aim angle from obj+(d0w, d1w) toward player A5+0x1D28. */
static uint16_t sub_0CC33A(const f3mem_t *m, uint32_t obj, uint16_t d0w, uint16_t d1w,
                           uint32_t *a1_out)
{
    uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);
    if (tgt == 0u) {
        *a1_out = 0xFFFFFFFFu;
        return 0x0080u;
    }
    *a1_out = tgt;
    int16_t cx = (int16_t)((int16_t)rd16(m, tgt + 0x2Cu) + (int16_t)rd16(m, tgt + 0x2Eu)) >> 1;
    int16_t cy = (int16_t)((int16_t)rd16(m, tgt + 0x30u) + (int16_t)rd16(m, tgt + 0x32u)) >> 1;
    uint16_t dx = (uint16_t)((uint16_t)(0u - d0w) + (uint16_t)cx + rd16(m, tgt + 0x14u) - rd16(m, obj + 0x14u));
    uint16_t dy = (uint16_t)((uint16_t)(0u - d1w) + (uint16_t)cy + rd16(m, tgt + 0x18u) - rd16(m, obj + 0x18u));
    return Obj_Atan2(m, dx, dy);
}

/* ROM 0x0CC2BC: frame-rate-compensated modulo timer -> D0w = rem, D0hi = quot. */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    d1 = (d1 >> 16) | (d1 << 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t d0 = rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(d0 / (uint16_t)d1);
    uint16_t rem = (uint16_t)(d0 % (uint16_t)d1);
    *d0_io = ((uint32_t)quot << 16) | rem;
    *d1_io = d1;
}

/* ROM 0x11AE62: drain +0x24 into shared HP pool A5+0x30DA. */
static void sub_11AE62(const f3mem_t *m, uint32_t obj,
                       uint32_t d0, uint32_t d1, uint32_t *a1_io)
{
    if ((int32_t)rd32(m, F3_A5 + 0x30DAu) <= 0)
        return;
    uint32_t dmg = rd32(m, obj + 0x24u);
    if (dmg == 0u)
        return;
    wr32(m, obj + 0x24u, 0u);
    uint32_t a1 = rd32(m, obj + 0x36u);
    *a1_io = a1;
    uint8_t c = rd8(m, a1 + 0x22u);
    if (c != 0u && c != 4u) {
        dmg = (uint32_t)((int32_t)dmg >> (d1 & 63u));
        if (c != 1u && c != 5u)
            dmg = (uint32_t)((int32_t)dmg >> 1);
    }
    uint32_t hp = rd32(m, F3_A5 + 0x30DAu) + dmg;
    wr32(m, F3_A5 + 0x30DAu, hp);
    if ((int32_t)hp <= 0) {
        a1 = Obj_0CBDDE(m, obj);
        *a1_io = a1;
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x11C096: queue hit-spark effect in 0x40B0F6..0x40B115 when +0x35 != 0. */
static void sub_11C096(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 0u)
        goto done;

    uint32_t a1 = rd32(m, obj + 0x08u);
    *a1_io = a1;
    wr8(m, a1 + 0x35u, (uint8_t)(rd8(m, a1 + 0x35u) | hit));

    if ((rd8(m, obj + 0x34u) & 0x02u) != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u) | hit;
        goto done;
    }

    {
        int16_t dt = (int16_t)(rd16(m, F3_A5 + 0x0878u) - rd16(m, F3_A5 + 0x311Cu));
        wr16(m, F3_A5 + 0x311Cu, rd16(m, F3_A5 + 0x0878u));
        *d0_io = setw(*d0_io, (uint16_t)dt);
        if (dt < 2) {
            uint32_t prev = rd32(m, F3_A5 + 0x3116u);
            *d0_io = prev;
            wr32(m, F3_A5 + 0x3116u, obj);
            if (prev == obj) {
                uint16_t cnt = (uint16_t)(rd16(m, F3_A5 + 0x311Au) + 1u);
                wr16(m, F3_A5 + 0x311Au, cnt);
                if ((int16_t)cnt <= 3)
                    goto done;
            }
        }
    }

    wr16(m, F3_A5 + 0x311Au, 0u);
    uint32_t a2 = 0x0040B0F6u;
    int found = 0;
    uint16_t d0w;
    for (d0w = 7; ; d0w--) {
        uint8_t b = rd8(m, a2);
        wr8(m, a2, (uint8_t)(b | 0x80u));
        if ((b & 0x80u) == 0u) {
            found = 1;
            break;
        }
        a2 += 4u;
        if (d0w == 0u)
            break;
    }
    *d0_io = setw(*d0_io, found ? d0w : 0xFFFFu);
    if (!found)
        goto done;

    a1 = rd32(m, obj + 0x36u);
    *a1_io = a1;
    uint16_t px, py;
    if (rd8(m, a1 + 0x22u) == 6u || rd8(m, obj + 0x35u) == 2u) {
        uint32_t r1 = Rng_NextMem(m);
        uint16_t h = (uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u));
        uint16_t rem1 = (uint16_t)((uint32_t)(uint16_t)r1 % h);
        py = (uint16_t)(rem1 + rd16(m, obj + 0x30u) + rd16(m, obj + 0x18u));

        uint32_t r0 = Rng_NextMem(m);
        uint16_t w = (uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
        uint16_t quot0 = (uint16_t)((uint32_t)(uint16_t)r0 / w);
        uint16_t rem0 = (uint16_t)((uint32_t)(uint16_t)r0 % w);
        px = (uint16_t)(rem0 + rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x14u));

        *d0_io = ((uint32_t)rem0 << 16) | px;
        *d1_io = ((uint32_t)quot0 << 16) | py;
    } else {
        int16_t x0 = (int16_t)(rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x14u));
        int16_t x0a = (int16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x14u));
        if (x0 < x0a)
            x0 = x0a;
        int16_t x1 = (int16_t)(rd16(m, obj + 0x2Eu) + rd16(m, obj + 0x14u));
        int16_t x1a = (int16_t)(rd16(m, a1 + 0x2Eu) + rd16(m, a1 + 0x14u));
        if (x1 > x1a)
            x1 = x1a;
        px = (uint16_t)((int16_t)(x0 + x1) >> 1);

        int16_t y0 = (int16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x18u));
        int16_t y0a = (int16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x18u));
        if (y0 < y0a)
            y0 = y0a;
        int16_t y1 = (int16_t)(rd16(m, obj + 0x32u) + rd16(m, obj + 0x18u));
        int16_t y1a = (int16_t)(rd16(m, a1 + 0x32u) + rd16(m, a1 + 0x18u));
        if (y1 > y1a)
            y1 = y1a;
        py = (uint16_t)((int16_t)(y0 + y1) >> 1);
    }

    px = (uint16_t)(px - rd16(m, F3_A5 + 0x30D2u));
    py = (uint16_t)(py - rd16(m, F3_A5 + 0x30D6u));
    *d0_io = setw(*d0_io, px);
    *d1_io = setw(*d1_io, py);

    uint32_t v = rd32(m, a2);
    v = (v & 0x801FFFFFu) | (((uint32_t)px & 0x3FFu) << 21);
    v = (v & 0xFFE00FFFu) | (((uint32_t)py & 0x1FFu) << 12);
    wr32(m, a2, v);

done:
    wr8(m, obj + 0x35u, 0u);
}

/* ROM 0x12B41E: set sprite, flip, hitboxes and draw offset from +0x20 angle. */
static void sub_12B41E(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    d0w = (uint16_t)((d0w & 0xFF00u) | (uint8_t)(d0w + 4u));
    d0w >>= 3;
    uint32_t d1 = (d0w & 0x0010u) ? 0xFFFFFFFFu : 0u;
    d0w &= 0x000Fu;

    uint32_t a1 = rd32(m, obj + 0x3Au);
    uint32_t a2 = rd32(m, a1);
    a1 += 4u;
    a1 += (uint32_t)(int32_t)(int16_t)d0w * 8u;
    a2 += (uint32_t)(int32_t)(int16_t)d0w * 4u;

    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a2++));

    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr8(m, obj + 0x1Cu, rd8(m, a1++));
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1++);
    wr16(m, obj + 0x52u, d0w);

    if ((uint16_t)d1 != 0u) {
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x03u));
        wr16(m, obj + 0x50u, (uint16_t)(-(int16_t)rd16(m, obj + 0x50u)));
        wr16(m, obj + 0x52u, (uint16_t)(-(int16_t)rd16(m, obj + 0x52u)));
    }

    *d0_io = setw(*d0_io, d0w);
    *d1_io = d1;
    *a1_out = a1;
}

/* ROM 0x12B48C: adjust (+0x18, +0x14) by sprite header * 8 - (+0x52, +0x50). */
static void sub_12B48C(const f3mem_t *m, uint32_t obj, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0 = (uint16_t)((rd16(m, a1) << 3) - rd16(m, obj + 0x52u));
    a1 += 2u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
    d0 = (uint16_t)((rd16(m, a1) << 3) - rd16(m, obj + 0x50u));
    a1 += 2u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    *a1_out = a1;
}

/* ROM 0x12A77E..0x12A7C8 / 0x12A9A4..0x12A9EC: compute packed spawn arg D2. */
static uint32_t pack_muzzle_arg(const f3mem_t *m, uint32_t obj, uint32_t d2_in,
                                uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    sub_12B48C(m, obj, a1_io);
    uint32_t tbl = 0x0012C3A2u;
    *a1_io = tbl;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    idx = (uint16_t)((idx & 0xFF00u) | (uint8_t)(idx + 4u));
    idx >>= 3;
    uint16_t vy = rd16(m, tbl + (uint32_t)(int32_t)(int16_t)idx * 4u + 2u);
    uint16_t vx = rd16(m, tbl + (uint32_t)(int32_t)(int16_t)idx * 4u);
    *d0_io = setw(*d0_io, vx);
    *d1_io = setw(*d1_io, vy);

    uint32_t d2 = d2_in;
    d2 = (d2 & 0x00FFFFFFu) | (((uint32_t)vx & 0xFFu) << 24);
    d2 = (d2 & 0xFF00FFFFu) | (((uint32_t)vy & 0xFFu) << 16);
    uint16_t d2w = rd16(m, obj + 0x20u);
    if ((rd8(m, obj + 0x06u) & 0x01u) != 0u)
        d2w = 0x0800u;
    d2w = (uint16_t)(d2w + 0x6000u);
    return (d2 & 0xFFFF0000u) | d2w;
}

/* ROM 0x12ADBC[0..14]: phase setup handlers 0x12AF34..0x12B178. */
static int run_phase_setup(const f3mem_t *m, uint32_t obj, int16_t phase,
                           uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    switch (phase) {
    case 0:
        wr16(m, F3_A5 + 0x3284u, 0x0A00u);
        wr16(m, F3_A5 + 0x3286u, 0xFF00u);
        wr16(m, F3_A5 + 0x328Au, 0x1080u);
        wr16(m, F3_A5 + 0x328Cu, 0x0300u);
        wr16(m, obj + 0x42u, 0x0000u);
        wr16(m, F3_A5 + 0x3280u, 0x0000u);
        if ((rd8(m, F3_A5 + 0x0879u) & 0x20u) == 0u) {
            wr16(m, F3_A5 + 0x3282u, 0x2C00u);
            wr16(m, F3_A5 + 0x3288u, 0x3000u);
        } else {
            wr16(m, F3_A5 + 0x3282u, 0x1C00u);
            wr16(m, F3_A5 + 0x3288u, 0x1000u);
        }
        return 0;

    case 1:
        wr16(m, F3_A5 + 0x3282u, 0x3B00u);
        wr16(m, F3_A5 + 0x3284u, 0x0A00u);
        wr16(m, F3_A5 + 0x3286u, 0xFF00u);
        wr16(m, F3_A5 + 0x3288u, 0x8C00u);
        wr16(m, F3_A5 + 0x328Au, 0x0A80u);
        wr16(m, F3_A5 + 0x328Cu, 0x0000u);
        wr16(m, obj + 0x42u, 0xFFF8u);
        wr16(m, F3_A5 + 0x3280u, 0x0000u);
        return 0;

    case 2:
        wr16(m, F3_A5 + 0x3282u, 0x3200u);
        wr16(m, F3_A5 + 0x3284u, 0x0400u);
        wr16(m, F3_A5 + 0x3286u, 0x0000u);
        wr16(m, F3_A5 + 0x3288u, 0xE600u);
        wr16(m, F3_A5 + 0x328Au, 0x0000u);
        wr16(m, F3_A5 + 0x328Cu, 0xFE70u);
        wr16(m, obj + 0x42u, 0x004Au);
        wr16(m, F3_A5 + 0x3280u, 0x2000u);
        return 0;

    case 3:
        wr16(m, F3_A5 + 0x3282u, 0x0600u);
        wr16(m, F3_A5 + 0x3284u, 0x1000u);
        wr16(m, F3_A5 + 0x3286u, 0xFF00u);
        wr16(m, F3_A5 + 0x3288u, 0x7700u);
        wr16(m, F3_A5 + 0x328Au, 0xFE00u);
        wr16(m, F3_A5 + 0x328Cu, 0x0000u);
        wr16(m, obj + 0x42u, 0xFFF8u);
        wr16(m, F3_A5 + 0x3280u, 0x3000u);
        return 0;

    case 4:
        wr16(m, F3_A5 + 0x3282u, 0x1800u);
        wr16(m, F3_A5 + 0x3284u, 0x0A00u);
        wr16(m, F3_A5 + 0x3286u, 0x0100u);
        wr16(m, F3_A5 + 0x328Au, 0x0000u);
        wr16(m, F3_A5 + 0x328Cu, 0x0000u);
        wr16(m, obj + 0x42u, 0x0000u);
        wr16(m, F3_A5 + 0x3280u, 0x3000u);
        if ((rd8(m, F3_A5 + 0x0879u) & 0x20u) == 0u)
            wr16(m, F3_A5 + 0x3288u, 0x8400u);
        else
            wr16(m, F3_A5 + 0x3288u, 0x5C00u);
        return 0;

    case 5:
        wr16(m, F3_A5 + 0x3282u, 0x1300u);
        wr16(m, F3_A5 + 0x3284u, 0x1000u);
        wr16(m, F3_A5 + 0x3286u, 0xFF00u);
        wr16(m, F3_A5 + 0x3288u, 0x6E00u);
        wr16(m, F3_A5 + 0x328Au, 0x16A0u);
        wr16(m, F3_A5 + 0x328Cu, 0xFD80u);
        wr16(m, obj + 0x42u, 0x0000u);
        wr16(m, F3_A5 + 0x3280u, 0x3000u);
        return 0;

    case 6:
        wr16(m, F3_A5 + 0x3282u, 0x2800u);
        wr16(m, F3_A5 + 0x3284u, 0x0780u);
        wr16(m, F3_A5 + 0x3286u, 0x0000u);
        wr16(m, F3_A5 + 0x3288u, 0x7400u);
        wr16(m, F3_A5 + 0x328Au, 0x0B80u);
        wr16(m, F3_A5 + 0x328Cu, 0x0000u);
        wr16(m, obj + 0x42u, 0xFFE0u);
        wr16(m, F3_A5 + 0x3280u, 0x0000u);
        return 0;

    case 7: {
        wr16(m, F3_A5 + 0x3282u, 0x0C00u);
        wr16(m, F3_A5 + 0x3284u, 0x0A00u);
        wr16(m, F3_A5 + 0x3286u, 0xFF00u);
        wr16(m, F3_A5 + 0x328Au, 0x0800u);
        wr16(m, F3_A5 + 0x328Cu, 0x0000u);
        wr16(m, obj + 0x42u, 0xFFF8u);
        wr16(m, F3_A5 + 0x3280u, 0x1000u);
        uint16_t ang = sub_0CC33A(m, rd32(m, F3_A5 + 0x3292u),
                                  (uint16_t)*d0_io, (uint16_t)*d1_io, a1_io);
        uint16_t res = (uint16_t)(((uint8_t)(ang - 0x46u)) << 8);
        *d0_io = setw(*d0_io, res);
        wr16(m, F3_A5 + 0x3288u, res);
        return 0;
    }

    case 8: {                                           /* 0x12B108 */
        wr16(m, F3_A5 + 0x3282u, 0x2300u);
        wr16(m, F3_A5 + 0x3284u, 0x0C00u);
        wr16(m, F3_A5 + 0x3286u, 0xFF00u);
        wr16(m, F3_A5 + 0x3288u, 0x7400u);
        wr16(m, F3_A5 + 0x328Cu, 0x0300u);
        wr16(m, obj + 0x42u, 0x0000u);
        wr16(m, F3_A5 + 0x3280u, 0x0000u);
        uint32_t a1 = rd32(m, F3_A5 + 0x30E2u);
        *a1_io = a1;
        uint16_t dx = (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + 0xFF9Du - rd16(m, a1 + 0x14u));
        uint16_t dy = (uint16_t)(rd16(m, F3_A5 + 0x30D6u) + 0xFFF2u - rd16(m, a1 + 0x18u));
        uint32_t dist = dist_full(m, dx, dy);
        uint16_t d0w = (uint16_t)((uint16_t)dist - 0x0010u);
        if ((int16_t)d0w < 0)
            d0w = 0u;
        d0w = (uint16_t)(0u - d0w);
        uint16_t d1w = d0w;
        d0w = (uint16_t)((d0w << 2) + 0xFE00u);
        wr16(m, F3_A5 + 0x328Au, d0w);
        d1w = (uint16_t)((d1w << 2) + 0x0300u);
        wr16(m, F3_A5 + 0x328Cu, d1w);
        *d0_io = setw(dist, d0w);
        *d1_io = setw(*d1_io, d1w);
        return 0;
    }

    case 9: case 10: case 11: case 12: case 13: case 14: /* 0x12B178: bare rts */
        return 0;

    default:
        return -1;
    }
}

int Fn_12A666(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x12A666 */

    {
        uint8_t flag = rd8(m, a1 + 0x59u);              /* 0x12A66A */
        wr8(m, obj + 0x59u, flag);
        if (flag != 0u) {                               /* 0x12A670 */
            Obj_Free(m, obj);                           /* 0x12AA0C */
            r->a[1] = a1;
            return 0;
        }
    }

    d0 = 0x000186A0u;                                   /* 0x12A674 */
    d1 = 2u;                                            /* 0x12A67A */
    sub_11AE62(m, obj, d0, d1, &a1);                    /* 0x12A67C */

    {
        int16_t phase = (int16_t)(int8_t)rd8(m, F3_A5 + 0x30F2u); /* 0x12A682 */
        d0 = setw(d0, (uint16_t)phase);
        a1 = rd32(m, 0x0012ADBCu + (uint32_t)(int32_t)phase * 4u);
        if (run_phase_setup(m, obj, phase, &d0, &d1, &a1) < 0)
            return -1;
    }

    {
        int16_t diff = (int16_t)(rd16(m, F3_A5 + 0x3280u) - rd16(m, obj + 0x20u)); /* 0x12A694 */
        if (diff > 0)
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0100u));
        else if (diff < 0)
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0100u));

        diff = (int16_t)(rd16(m, obj + 0x42u) - rd16(m, F3_A5 + 0x327Eu)); /* 0x12A6B6 */
        if (diff > 0)
            wr16(m, F3_A5 + 0x327Eu, (uint16_t)(rd16(m, F3_A5 + 0x327Eu) + 1u));
        else if (diff < 0)
            wr16(m, F3_A5 + 0x327Eu, (uint16_t)(rd16(m, F3_A5 + 0x327Eu) - 1u));
    }

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) < 0x01F4) {   /* 0x12A6D4 */
        sub_11C096(m, obj, &d0, &d1, &a1);              /* 0x12A6DE */
        a1 = rd32(m, obj + 0x08u);                      /* 0x12A6E4 */
        wr8(m, obj + 0x4Fu, rd8(m, a1 + 0x1Du));        /* 0x12A6E8 */
        wr16(m, obj + 0x20u, 0x7800u);                  /* 0x12A6EE */
        uint8_t s6 = rd8(m, obj + 0x06u);               /* 0x12A6F4 */
        wr8(m, obj + 0x06u, (uint8_t)(s6 | 0x01u));
        if ((s6 & 0x01u) == 0u) {                       /* 0x12A6FA */
            d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 5u); /* 0x12A710 */
            Sound_PlayHooked(m, 0x16u);                 /* 0x12A71A */
            Sound_PlayHooked(m, 0x14u);                 /* 0x12A72E */
        }
    }

    sub_12B41E(m, obj, &d0, &d1, &a1);                  /* 0x12A742 */
    Obj_11B06E(m, obj);                                 /* 0x12A746 */
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u)
        a1 = rd32(m, F3_A5 + 0x0906u);

    wr16(m, obj + 0x14u, rd16(m, F3_A5 + 0x30D2u));     /* 0x12A74C */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, F3_A5 + 0x30D6u) - 0x0040u)); /* 0x12A758..0x12A75E */

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x20u) != 0u) {      /* 0x12A764: bit 2 */
        sub_0CC2BC(m, 0x0010u, &d0, &d1);
        if ((uint16_t)d0 == 0u) {
            uint16_t sx = rd16(m, obj + 0x14u);
            uint16_t sy = rd16(m, obj + 0x18u);
            d2 = pack_muzzle_arg(m, obj, d2, &d0, &d1, &a1);
            wr16(m, obj + 0x18u, sy);
            wr16(m, obj + 0x14u, sx);
            (void)Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, d2);
            d2 = setw(d2, (uint16_t)((uint16_t)d2 - 0x0800u));
            (void)Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, d2);
            d2 = setw(d2, (uint16_t)((uint16_t)d2 + 0x1000u));
            d0 = Obj_Spawn(m, obj, 0x000CD124u, 9u, 2u, d2);
        }
    }

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) != 0u) {      /* 0x12A81E: bit 0 */
        sub_0CC2BC(m, 0x0020u, &d0, &d1);
        if ((uint16_t)d0 == 0u) {
            d0 = (d0 >> 16) | (d0 << 16);
            d0 = (d0 & 0xFFFF0000u) | ((uint16_t)d0 & 7u);
            if ((int16_t)(uint16_t)d0 < 3) {
                static const uint32_t args[4] = {
                    0x471EE000u, 0x4A300000u, 0x413D2000u, 0x3A444000u
                };
                for (int i = 0; i < 4; i++) {
                    d1 = args[i];
                    d0 = Obj_Spawn(m, obj, 0x0011B5E6u, 5u, 2u, d1);
                }
            }
        }
    }

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) >= 0x01F4) {  /* 0x12A8C6 */
        uint8_t b = rd8(m, F3_A5 + 0x30EEu);
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(b & ~0x02u)); /* 0x12A8D0: bit 6 */
        int fire = (b & 0x02u) != 0u;
        if (!fire) {
            if ((b & 0x04u) == 0u) {                    /* 0x12A8DA: bit 5 */
                wr8(m, obj + 0x07u, 0u);
            } else if (rd8(m, obj + 0x07u) != 0u) {
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) - 1u));
            } else {
                fire = 1;
            }
        }
        if (fire) {                                     /* 0x12A8F4 */
            d1 = 0x42413C00u;
            (void)Obj_Spawn(m, obj, 0x0012B934u, 9u, 2u, d1);
            static const uint16_t lows[4] = { 0x4200u, 0x4800u, 0x4E00u, 0x5400u };
            for (int i = 0; i < 4; i++) {
                d1 = setw(d1, lows[i]);
                d0 = Obj_Spawn(m, obj, 0x0012B934u, 9u, 2u, d1);
            }
            wr8(m, obj + 0x07u, 0x64u);
        }
    } else {
        wr8(m, obj + 0x07u, 0u);                        /* 0x12A996 */
    }

    {
        uint8_t b = rd8(m, F3_A5 + 0x30EFu);
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(b & ~0x08u)); /* 0x12A99A: bit 12 */
        if ((b & 0x08u) != 0u) {
            uint16_t sx = rd16(m, obj + 0x14u);
            uint16_t sy = rd16(m, obj + 0x18u);
            d2 = pack_muzzle_arg(m, obj, d2, &d0, &d1, &a1);
            wr16(m, obj + 0x18u, sy);
            wr16(m, obj + 0x14u, sx);
            d0 = Obj_Spawn(m, obj, 0x0012BDB2u, 9u, 2u, d2);
        }
    }

    wr32(m, obj, 0x0012A666u);                          /* 0x12AA08 -> 0x12A65E */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
