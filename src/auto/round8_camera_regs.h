#ifndef ROUND8_CAMERA_REGS_H
#define ROUND8_CAMERA_REGS_H

#include "fn_call_frames.h"
#include "../objects/obj_shared.h"

/* ROM 0x0CBE12: integrate the position at the angle in D0.W. */
static inline void move_by_angle_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int32_t index = (int16_t)r->d[0] * 2;
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);

    r->a[3] = 0x027E40u;
    r->d[1] = (uint32_t)(speed * (int16_t)rd16(m, r->a[3] + 0x80u + index));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + r->d[1]);
    r->d[2] = (uint32_t)(speed * (int16_t)rd16(m, r->a[3] + index));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + r->d[2]);
}

/* ROM 0x11B288: turn toward the target, with the D0 upper word from
 * 0x006422 (EXG in the mixed-sign quadrants). */
static inline void steer_camera(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    int16_t dx = (int16_t)(rd16(m, obj + 0x50u) - rd16(m, a5 + 0x30D2u));
    int16_t dy = (int16_t)(rd16(m, obj + 0x52u) - rd16(m, a5 + 0x30D6u));
    r->d[1] = set_word(r->d[1], (uint16_t)dy);

    uint32_t sp = r->a[7];
    push_long(m, r, 0x11B29Eu);
    push_long(m, r, obj);
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    uint8_t angle = (uint8_t)Obj_Atan2(m, (uint16_t)dx, (uint16_t)dy);
    r->a[7] = sp;

    uint8_t current = rd8(m, obj + 0x20u);
    int8_t delta = (int8_t)(angle - current);
    if (delta > 2)
        delta = 2;
    else if (delta < -2)
        delta = -2;
    wr8(m, obj + 0x20u, (uint8_t)(current + delta));
    uint32_t high = (dx < 0) != (dy < 0) ? r->d[1] : r->d[0];
    r->d[0] = (high & 0xFFFF0000u) | (uint8_t)delta;
}

/* ROM 0x11B2CA: ease speed toward the target and publish its integer part. */
static inline void ease_camera_speed(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int8_t target = (int8_t)rd8(m, obj + 0x54u);
    uint16_t speed = rd16(m, obj + 0x56u);
    int8_t current = (int8_t)(speed >> 8);

    if (target != current) {
        speed = (uint16_t)(speed + (target > current ? 0x20 : -0x10));
        wr16(m, obj + 0x56u, speed);
    }
    uint16_t integer_speed = (uint16_t)(int16_t)(int8_t)(speed >> 8);
    r->d[0] = set_word(r->d[0], integer_speed);
    wr16(m, obj + 0x42u, integer_speed);
}

/* ROM 0x11B262: load the camera coordinates, integrate, and copy back. */
static inline void integrate_camera(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    wr32(m, obj + 0x14u, rd32(m, a5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, a5 + 0x30D6u));
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    push_long(m, r, 0x11B27Au);
    move_by_angle_regs(m, r);
    r->a[7] += 4u;
    wr32(m, a5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, a5 + 0x30D6u, rd32(m, obj + 0x18u));
}

/* The camera states start with the same three absolute JSRs and SUBQ.W. */
static inline int tick_camera(const f3mem_t *m, f3regs_t *r, uint32_t entry)
{
    push_long(m, r, entry + 6u);
    steer_camera(m, r);
    r->a[7] += 4u;

    push_long(m, r, entry + 12u);
    ease_camera_speed(m, r);
    r->a[7] += 4u;

    push_long(m, r, entry + 18u);
    integrate_camera(m, r);
    r->a[7] += 4u;

    uint32_t timer_addr = r->a[0] + 6u;
    uint16_t timer = rd16(m, timer_addr);
    wr16(m, timer_addr, (uint16_t)(timer - 1u));
    /* BGT takes the overflow flag from SUBQ.W into account. */
    return (int16_t)timer > 1;
}

#endif
