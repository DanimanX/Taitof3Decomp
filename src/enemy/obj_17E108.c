#include "obj_17E108.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* Animation state machines, table 0x179A5C (init, tick) per index (+0x40).
 * Only the indices these objects use are translated; others return -1. */

/* ROM 0x179550 (index 0 init) */
static void anim0_init(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
}

/* ROM 0x1798D4 (index 10 tick): pick one of 32 direction frames
 * {frame.l, colour.w} from +0x3A by the heading byte +0x20 */
static void anim10_tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
    uint32_t a1 = rd32(m, obj + 0x3Au) + (uint32_t)(d0 * 6u);
    wr32(m, obj + 0x10u, rd32(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
}

/* ROM 0x179614 (index 10 init) */
static void anim10_init(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);
    anim10_tick(m, obj);
}

static int anim_init(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u:  anim0_init(m, obj); return 0;
    case 10u: anim10_init(m, obj); return 0;
    default:  return -1;
    }
}

/* Load script at a1 ({index.w, data...}) and run its init (jsr via table). */
static int anim_start(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1 + 2u);
    return anim_init(m, obj, d0);
}

static void dec3e(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
}

/* ROM 0x179664 (index 0 tick): frames of 6 bytes */
static int anim0_tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)rd16(m, obj + 0x3Eu);
    if (t < 0)
        return 0;
    if (t == 0) {
        uint32_t a1 = rd32(m, obj + 0x3Au) + 6u;
        int16_t w = (int16_t)rd16(m, a1);
        if (w > 0) {
            wr32(m, obj + 0x3Au, a1);
            anim0_init(m, obj);
        } else if (w == 0) {
            if (anim_start(m, obj, rd32(m, a1 + 2u)) < 0)
                return -1;
        }
    }
    dec3e(m, obj);
    return 0;
}

static int anim_tick(const f3mem_t *m, uint32_t obj)
{
    switch (rd16(m, obj + 0x40u)) {
    case 0u:  return anim0_tick(m, obj);
    case 10u: anim10_tick(m, obj); return 0;
    default:  return -1;
    }
}

/* +0x10 frame: {dy.w, dx.w}; centre the sprite on +0x14/+0x18 */
static void sprite_centre(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
    uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
}

/* ROM 0x006422: atan2-style direction (0..0xFF) of (d0, d1) */
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
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
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
    d0 = (uint16_t)(d0 + rd16(m, 0x006492u + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x178B20: +0x42 = entry.w[6] */
static void scr_178B20(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, rd16(m, rd32(m, obj + 0x44u) + 6u));
}

/* ROM 0x178E4A: home on the target (+0x4C): turn +0x20 by entry.w[8] towards
 * it, accelerate +0x42 by entry.w[6], move +0x50/+0x54 along +0x20 */
static void scr_178E4A(const f3mem_t *m, uint32_t obj)
{
    uint32_t tgt = rd32(m, obj + 0x4Cu);
    if ((int32_t)tgt <= 0)
        return;
    uint16_t dx = (uint16_t)(rd16(m, tgt + 0x14u) + 0x18u - rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(rd16(m, tgt + 0x18u) + 0x10u - rd16(m, obj + 0x54u));
    uint16_t d0 = sub_006422(m, dx, dy);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d2 = rd16(m, a1 + 6u);
    uint16_t d1 = rd16(m, a1 + 8u);
    int8_t diff = (int8_t)(uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
    if (diff == 0)
        d1 = 0u;
    else if (diff < 0)
        d1 = (uint16_t)(0u - d1);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + d2));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d1));
    int16_t sp = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u);
    sp = (int16_t)(sp >> 6);
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int32_t vx = (int32_t)sp * (int16_t)rd16(m, 0x00027E40u + 0x80u + ang * 2u);
    int32_t vy = (int32_t)sp * (int16_t)rd16(m, 0x00027E40u + ang * 2u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

static int scr_call(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    switch (fn) {
    case 0x00178B20u: scr_178B20(m, obj); return 0;
    case 0x00178E4Au: scr_178E4A(m, obj); return 0;
    default:          return -1;
    }
}

/* ROM 0x178982: movement script at +0x44: entries {fn.l, frames.w, arg.l};
 * fn = 0 / negative -> loop marker (count +0x4A, target arg). */
static int sub_178982(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    for (;;) {
        a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);
        if (fn == 0u)
            goto loop_marker;
        if (scr_call(m, obj, fn) < 0)
            return -1;
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
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    return 0;
}

/* ROM 0x17B02E: score receiver: 1 -> (+0x36)->+8, 2 -> +0x36, else -1 */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u)
        return rd32(m, rd32(m, obj + 0x36u) + 8u);
    if (k == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x17C614 */
static int lab_17C614(const f3mem_t *m, uint32_t obj)
{
    if (sub_178982(m, obj) < 0)
        return -1;
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (anim_tick(m, obj) < 0)
        return -1;
    sprite_centre(m, obj);
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0017C660u);
    return 0;
}

/* ROM 0x17C662: shrink away (+0x1E/+0x1F zoom) */
static int lab_17C662(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x1Fu) >= 0xF0u || rd8(m, obj + 0x1Eu) >= 0xF0u) {
        Obj_Free(m, obj);
        return 0;
    }
    if (sub_178982(m, obj) < 0)
        return -1;
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x10u));
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 2u) << 4);
    uint16_t d0 = (uint16_t)(((uint32_t)rd8(m, obj + 0x1Fu) * d1) & 0xFFFFu);
    d0 = (uint16_t)((d0 >> 8) >> 1);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));
    a1 = rd32(m, obj + 0x10u);
    d1 = (uint16_t)(rd16(m, a1) << 4);
    d0 = (uint16_t)(((uint32_t)rd8(m, obj + 0x1Eu) * d1) & 0xFFFFu);
    d0 = (uint16_t)((d0 >> 8) >> 1);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
    if (anim_tick(m, obj) < 0)
        return -1;
    sprite_centre(m, obj);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0017C700u);
    return 0;
}

/* ROM 0x17C59A: destroyed: award score, start the explosion animation */
static int lab_17C59A(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x34u) & 0x02u)
        return lab_17C662(m, obj);
    uint32_t d1 = rd16(m, obj + 0x62u);
    uint32_t rcv = sub_17B02E(m, obj);
    if ((int32_t)rcv >= 0)
        wr32(m, rcv + 0x48u, rd32(m, rcv + 0x48u) + d1);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    int16_t d = (int16_t)(uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
    if (anim_start(m, obj, d > 8 ? 0x0017C7DCu : 0x0017C79Cu) < 0)
        return -1;
    return lab_17C614(m, obj);
}

/* ROM 0x17E108: wait +6 frames after the death fragments, then explode */
int Obj_17E108_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0017E108u);
        return 0;
    }
    return lab_17C59A(m, obj);
}
