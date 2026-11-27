#ifndef ROUND8_PROJECTILE_REGS_H
#define ROUND8_PROJECTILE_REGS_H

#include "batch_obj_regs.h"
#include "fn_sound_frames.h"
#include "round8_camera_regs.h"

/* ROM 0x001282: the vertical-flip setup at 0x0012DC/0x0012FC leaves
 * the upper half of the signed row offset in D2 throughout the draw. */
static inline void projectile_sprite_outputs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sprite = rd32(m, r->a[0] + 0x10u);
    batch_sprite_outputs(m, r);
    if (rd32(m, sprite) != 0x00010001u && (rd8(m, r->a[0] + 0x1Cu) & 1u)) {
        int16_t rows_minus_one = (int16_t)(rd16(m, sprite + 2u) - 1u);
        int16_t row_step = (int16_t)r->d[7];
        uint32_t offset = (uint32_t)(rows_minus_one * row_step);
        r->d[2] = set_word(offset, (uint16_t)r->d[2]);
    }
}

static inline void draw_projectile(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    Obj_DrawSprite(m, r->a[0]);
    projectile_sprite_outputs(m, r);
    r->a[7] += 4u;
}

/* ROM 0x0CBDAE, also inlined at 0x12BA1A. */
static inline void register_projectile(const f3mem_t *m, f3regs_t *r)
{
    uint32_t cursor = r->a[5] + 0x906u;
    r->a[1] = rd32(m, cursor);
    wr32(m, r->a[1], r->a[0]);
    r->a[1] += 4u;
    wr32(m, cursor, r->a[1]);
}

/* CMP2.W with a data-register operand compares signed words. */
static inline int outside_word_bounds(const f3mem_t *m, uint32_t bounds, int16_t value)
{
    return value < (int16_t)rd16(m, bounds) || value > (int16_t)rd16(m, bounds + 2u);
}

/* ROM 0x00649A: preserve both halves of the scaled distance after SWAP. */
static inline uint32_t projectile_distance(const f3mem_t *m, int16_t x, int16_t y)
{
    uint32_t sum = ((uint32_t)(x * x) + (uint32_t)(y * y)) * 2u;
    unsigned shift = 4u;
    while (shift < 16u && (int32_t)sum >= 0x200) {
        sum >>= 8;
        shift += 4u;
    }
    uint32_t scaled = set_word(sum, rd16(m, 0x028340u + (int16_t)sum * 2));
    scaled <<= shift;
    return (scaled << 16) | (scaled >> 16);
}

/* ROM 0x0CBB2C: the shared explosion tick performs the object/sprite writes.
 * Reproduce the registers and scratch stack on the three exits. */
static inline void explode_projectile(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7];
    uint32_t sprite = rd32(m, obj + 0x10u);
    int16_t h = (int16_t)rd16(m, sprite);
    int16_t w = (int16_t)rd16(m, sprite + 2u);
    int16_t off_y = (int16_t)((rd8(m, obj + 0x1Eu) - 256) * h);
    int16_t off_x = (int16_t)((rd8(m, obj + 0x1Fu) - 256) * w);
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) - (off_x >> 5) - 0xA0);
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) - (off_y >> 5) - 0x74);

    r->a[1] = sprite + 4u;
    r->d[1] = set_word(r->d[1], (uint16_t)y);
    push_long(m, r, 0x0CBB92u);
    push_long(m, r, obj);
    push_long(m, r, r->d[1]);
    r->a[7] = sp;

    uint32_t distance = projectile_distance(m, x, y);
    uint16_t radius = (uint16_t)(distance << 2);
    uint16_t threshold = rd16(m, r->a[5] + 0x1D5Cu);
    r->d[0] = set_word(distance, (uint16_t)(radius - threshold));
    int reaches_angle = (int16_t)radius >= (int16_t)threshold;
    if (reaches_angle) { /* 0x0CBBA2..0x0CBBB8 */
        push_long(m, r, 0x0CBBB8u);
        push_long(m, r, obj);
        push_long(m, r, r->d[3]);
        push_long(m, r, r->d[2]);
        push_long(m, r, r->d[1]);
        r->a[7] = sp;
        uint32_t high = (x < 0) != (y < 0) ? r->d[1] : distance;
        r->d[0] = set_word(high, (uint16_t)(Obj_Atan2(m, (uint16_t)x, (uint16_t)y) << 8));
    }

    int draws = reaches_angle && (rd8(m, r->a[5] + 0x1D58u) & 2u);
    if (draws)
        push_long(m, r, 0x0CBC70u);
    Obj_ExplodeTick(m, obj);
    if (draws) {
        /* 0x0CBC0C..0x0CBC68 immediately before the draw at 0x0CBC6A. */
        uint16_t angle = rd16(m, obj + 0x2Eu) >> 8;
        int16_t radius_with_base = (int16_t)((rd16(m, obj + 0x2Cu) >> 6) + threshold);
        r->d[1] = (uint32_t)(radius_with_base *
                   (int16_t)rd16(m, 0x027EC0u + angle * 2u)) + 0x00A00000u;
        r->d[2] = (uint32_t)(radius_with_base *
                   (int16_t)rd16(m, 0x027E40u + angle * 2u)) + 0x00740000u;
        r->d[0] = (uint32_t)((rd8(m, obj + 0x1Eu) - 256) * h);
        r->d[0] = set_word(r->d[0], (uint16_t)((int16_t)r->d[0] >> 5));
        projectile_sprite_outputs(m, r);
        r->a[7] += 4u;
    }
}

#endif
