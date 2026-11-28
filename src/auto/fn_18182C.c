#include "fn_auto.h"
/* ROM 0x18182C: relay damage, update the part's sprite and collision, or start its destruction animation. */
#include "fn_sound_frames.h"
#include "fn_sprite_regs.h"

static uint32_t anim_dir_idx(const f3mem_t *m, uint32_t obj, int dirs32)
{
    uint8_t ang = rd8(m, obj + 0x20u);
    return dirs32 ? (uint32_t)((uint8_t)(ang + 4u) >> 3)
                  : (uint32_t)((uint8_t)(ang + 8u) >> 4);
}

static void anim_dir_frame(const f3mem_t *m, f3regs_t *r, uint32_t obj,
                          int dirs32, uint32_t stride,
                          int has_pal, int copy_boxes, int add_xy)
{
    uint32_t d0 = anim_dir_idx(m, obj, dirs32);
    uint32_t offset = d0 * stride;
    uint32_t a1 = rd32(m, obj + 0x3Au) + offset;
    wr32(m, obj + 0x10u, rd32(m, a1));
    if (stride != 4u) {
        d0 = offset;
        a1 += 4u;
    }
    if (has_pal) {
        uint16_t w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w);
        d0 = set_word(d0, w);
    }
    if (copy_boxes) {
        wr32(m, obj + 0x2Cu, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1));
        a1 += 4u;
    }
    if (add_xy) {
        d0 = (uint32_t)(int32_t)(int8_t)rd8(m, a1);
        uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
        r->d[1] = d1;
    }
    r->d[0] = d0;
    r->a[1] = a1;
}

static void anim_init_frame(const f3mem_t *m, f3regs_t *r, uint32_t obj,
                            int has_pal, int copy_boxes, int add_xy)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    if (has_pal) {
        uint16_t w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w);
        r->d[0] = set_word(r->d[0], w);
    }
    if (copy_boxes) {
        wr32(m, obj + 0x2Cu, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x30u, rd32(m, a1));
        a1 += 4u;
    }
    if (add_xy) {
        uint32_t d0 = (uint32_t)(int32_t)(int8_t)rd8(m, a1);
        uint32_t d1 = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1));
        r->d[0] = d0;
        r->d[1] = d1;
    }
    r->a[1] = a1;
}

/* 0x179A5C: both halves of the directional entries share one body. */
static void directional_frame(const f3mem_t *m, f3regs_t *r, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 1u:  anim_dir_frame(m, r, obj, 0,  4u, 0, 0, 0); return; /* 0x1795F6 */
    case 2u:  anim_dir_frame(m, r, obj, 1,  4u, 0, 0, 0); return; /* 0x17960A */
    case 4u:  anim_dir_frame(m, r, obj, 0, 12u, 0, 1, 0); return; /* 0x17961E */
    case 6u:  anim_dir_frame(m, r, obj, 0, 14u, 0, 1, 1); return; /* 0x179632 */
    case 7u:  anim_dir_frame(m, r, obj, 1, 14u, 0, 1, 1); return; /* 0x179646 */
    case 9u:  anim_dir_frame(m, r, obj, 0,  6u, 1, 0, 0); return; /* 0x179600 */
    case 10u: anim_dir_frame(m, r, obj, 1,  6u, 1, 0, 0); return; /* 0x179614 */
    case 12u: anim_dir_frame(m, r, obj, 0, 14u, 1, 1, 0); return; /* 0x179628 */
    case 14u: anim_dir_frame(m, r, obj, 0, 16u, 1, 1, 1); return; /* 0x17963C */
    case 15u: anim_dir_frame(m, r, obj, 1, 16u, 1, 1, 1); return; /* 0x179650 */
    case 16u: {                                              /* 0x17965A */
        int8_t b = (int8_t)(uint8_t)(rd8(m, obj + 0x20u) - 0x20u);
        if (b < 0) b = 0;
        if (b > 0x40) b = 0x40;
        uint32_t d0 = (uint32_t)((uint8_t)(b + 2) >> 2);
        uint32_t a1 = rd32(m, obj + 0x3Au) + d0 * 6u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        uint16_t w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w);
        r->d[0] = set_word(d0, w);
        r->a[1] = a1;
        return;
    }
    default:
        return;
    }
}

/* ROM 0x17B92C: scale the damage by the hit type and apply it to the parent. */
static void relay_damage(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint8_t hit = rd8(m, obj + 0x35);
    if (hit == 0)
        return;

    r->d[0] = rd32(m, obj + 0x24);
    uint32_t multiplier = 0;
    if (hit == 2) {
        multiplier = 0x4B2E;
    } else {
        r->a[1] = rd32(m, obj + 0x36);
        uint8_t phase = rd8(m, r->a[1] + 0x22);
        if (phase == 1 || phase == 2)
            multiplier = 0x4B2A + 2u * (phase - 1u);
    }
    if (multiplier != 0) {
        r->d[1] = rd16(m, r->a[5] + multiplier);
        r->d[0] = (uint32_t)((int32_t)r->d[0] / 100) * r->d[1];
    }

    r->a[4] = rd32(m, obj + 0x08); /* 0x17B9A0 */
    wr32(m, r->a[4] + 0x24, rd32(m, r->a[4] + 0x24) + r->d[0]);
    wr32(m, obj + 0x24, 0);
    wr32(m, r->a[4] + 0x36, rd32(m, obj + 0x36));
    wr16(m, r->a[4] + 0x34, rd16(m, obj + 0x34));
}

/* The timed entries in the table at 0x179A5C. */
static const struct {
    uint8_t size;
    uint8_t palette;
    uint8_t hitbox;
    uint8_t offset;
    uint32_t advance_ret;
    uint32_t reload_ret;
} timed_frames[17] = {
    [0]  = { 6,  0, 0, 0, 0x1796A0u, 0x1796ACu },
    [3]  = { 14, 0, 1, 0, 0x17973Cu, 0x179748u },
    [5]  = { 16, 0, 1, 1, 0x1797DAu, 0x1797E6u },
    [8]  = { 8,  1, 0, 0, 0x1796EEu, 0x1796FAu },
    [11] = { 16, 1, 1, 0, 0x17978Au, 0x179796u },
    [13] = { 18, 1, 1, 1, 0x17984Cu, 0x179858u },
};

static void init_frame(const f3mem_t *m, f3regs_t *r, uint16_t idx, uint32_t ret)
{
    uint32_t obj = r->a[0];
    push_long(m, r, ret);
    if (timed_frames[idx].size) {
        anim_init_frame(m, r, obj, timed_frames[idx].palette,
                        timed_frames[idx].hitbox, timed_frames[idx].offset);
    } else {
        wr16(m, obj + 0x3Eu, 1u);
        directional_frame(m, r, obj, idx);
    }
    r->a[7] += 4u;
}

/* 0x179664..0x17987E, or the directional tick selected by the same table. */
static void tick_frame(const f3mem_t *m, f3regs_t *r, uint16_t idx, uint32_t ret)
{
    uint32_t obj = r->a[0];
    push_long(m, r, ret);
    r->a[1] = rd32(m, 0x179A60u + 8u * idx);
    if (!timed_frames[idx].size) {
        directional_frame(m, r, obj, idx);
    } else {
        int16_t timer = (int16_t)rd16(m, obj + 0x3Eu);
        int apply_offset = timed_frames[idx].offset;
        if (timer == 0) {
            r->a[1] = rd32(m, obj + 0x3Au) + timed_frames[idx].size;
            int16_t duration = (int16_t)rd16(m, r->a[1]);
            if (duration > 0) {
                wr32(m, obj + 0x3Au, r->a[1]);
                init_frame(m, r, idx, timed_frames[idx].reload_ret);
                apply_offset = 0;
            } else if (duration == 0) {
                r->a[1] = rd32(m, r->a[1] + 2u);
                uint16_t next = rd16(m, r->a[1]);
                r->d[0] = set_word(r->d[0], next);
                r->a[1] += 2u;
                wr16(m, obj + 0x40u, next);
                wr32(m, obj + 0x3Au, r->a[1]);
                init_frame(m, r, next, timed_frames[idx].advance_ret);
                apply_offset = 0;
            }
        }
        if (timer >= 0)
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 1u));
        if (apply_offset) { /* 0x1797EE / 0x179860 */
            r->a[1] = rd32(m, obj + 0x3Au);
            uint32_t offset = r->a[1] + timed_frames[idx].size - 2u;
            r->d[0] = (uint32_t)(int32_t)(int8_t)rd8(m, offset);
            r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, offset + 1u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + r->d[0]));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + r->d[1]));
        }
    }
    r->a[7] += 4u;
}

/* 0x181848 / 0x181914: tick the current animation and center its sprite. */
static void center_sprite(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t obj = r->a[0];
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    uint16_t idx = rd16(m, obj + 0x40u);
    r->d[0] = set_word(r->d[0], idx);
    tick_frame(m, r, idx, ret);
    r->a[1] = rd32(m, obj + 0x10u);
    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, r->a[1]) << 3));
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, r->a[1] + 2u) << 3));
    r->a[1] += 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - r->d[0]));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - r->d[1]));
}

/* ROM 0x17B02E: resolve the object receiving the score. */
static void score_receiver(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 1u) {
        r->a[1] = rd32(m, obj + 0x36u);
        r->d[0] = rd32(m, r->a[1] + 8u);
    } else if (mode == 2u) {
        r->d[0] = rd32(m, obj + 0x36u);
    } else {
        r->d[0] = UINT32_MAX;
    }
}

/* Both entry points dispatch through the same animation table. */
static int valid_animation(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = rd16(m, obj + 0x40u);
    if (idx >= 17u)
        return 0;
    if (timed_frames[idx].size && rd16(m, obj + 0x3Eu) == 0u) {
        uint32_t next = rd32(m, obj + 0x3Au) + timed_frames[idx].size;
        if (rd16(m, next) == 0u)
            return rd16(m, rd32(m, next + 2u)) < 17u;
    }
    return 1;
}

/* ROM 0x181914: common tail of the destruction setup and its frame tick. */
static int draw_destroyed_part(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    center_sprite(m, r, 0x181930u);
    r->a[4] = rd32(m, obj + 8u);
    if (rd8(m, r->a[4] + 0x23u) & 4u) {
        call_free(m, r, 0x17C796u);                    /* 0x18194E -> 0x17C78E */
    } else {
        call_draw_sprite(m, r, 0x181958u);
        wr32(m, obj, 0x181960u);
    }
    r->a[7] += 4u;
    return 0;
}

int Fn_181914(const f3mem_t *m, f3regs_t *r)
{
    if (!valid_animation(m, r->a[0]))
        return -1;
    return draw_destroyed_part(m, r);
}

int Fn_18182C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int parent_removed = rd8(m, rd32(m, obj + 8u) + 0x23u) & 4u;
    int alive = (int32_t)rd32(m, obj + 0x28u) > 0;

    if (!parent_removed && alive && !valid_animation(m, obj))
        return -1;

    r->a[4] = rd32(m, obj + 8u);
    if (parent_removed)
        goto free_part;                                 /* 0x181836 -> 0x17C78E */

    if (alive) {
        push_long(m, r, 0x181848u);
        relay_damage(m, r);                              /* 0x181842 */
        r->a[7] += 4u;
        center_sprite(m, r, 0x181864u);
        uint8_t colour = 0xB8u;
        if (rd8(m, obj + 0x1Du) == 6u)
            colour = 8u;
        else if (rd8(m, obj + 0x35u))
            colour = 6u;
        wr8(m, obj + 0x1Du, colour);
        wr8(m, obj + 0x35u, 0u);                         /* 0x1818A8 */
        call_draw_sprite(m, r, 0x1818B2u);
        r->a[1] = rd32(m, r->a[5] + 0x8FEu);
        wr32(m, r->a[1], obj);
        r->a[1] += 4u;
        wr32(m, r->a[5] + 0x8FEu, r->a[1]);
        r->a[1] = rd32(m, r->a[5] + 0x906u);
        wr32(m, r->a[1], obj);
        r->a[1] += 4u;
        wr32(m, r->a[5] + 0x906u, r->a[1]);
        wr32(m, obj, 0x18182Cu);                         /* 0x181824 */
    } else {
        push_long(m, r, 0x1818D0u);
        score_receiver(m, r);
        r->a[7] += 4u;
        if ((int32_t)r->d[0] >= 0) {
            r->a[1] = r->d[0];
            wr32(m, r->a[1] + 0x48u, rd32(m, r->a[1] + 0x48u) + 1000u);
        }
        uint16_t idx = rd16(m, 0x17C81Cu);              /* 0x1818E0: index 0 */
        r->d[0] = set_word(r->d[0], idx);
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, 0x17C81Eu);
        init_frame(m, r, idx, 0x1818FCu);
        wr8(m, obj + 0x1Du, 0u);
        call_hooked_sound(m, r, 0x40u, 0x18190Au, 0x181910u);
        return draw_destroyed_part(m, r);               /* 0x181914 */
    }
    r->a[7] += 4u;
    return 0;

free_part:
    call_free(m, r, 0x17C796u);
    r->a[7] += 4u;
    return 0;
}
