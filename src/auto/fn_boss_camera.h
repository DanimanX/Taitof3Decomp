#ifndef FN_BOSS_CAMERA_H
#define FN_BOSS_CAMERA_H

#include "fn_call_frames.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

/* ROM 0x006422. The angle lookup preserves D1-D3/A0, while EXG at
 * 0x006474 can supply the upper word of D0 from D1. */
static inline void boss_angle(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_long(m, r, ret);
    push_long(m, r, r->a[0]);
    for (unsigned i = 3; i != 0; --i)
        push_long(m, r, r->d[i]);

    int16_t x = (int16_t)r->d[0];
    int16_t y = (int16_t)r->d[1];
    uint32_t upper = (x < 0) != (y < 0) ? r->d[1] : r->d[0];
    r->d[0] = set_word(upper, Obj_AtanByte(m, x, y));
    r->a[7] = sp;
}

/* ROM 0x0CBE12. Obj_MoveByAngle performs the two position writes. */
static inline void boss_move(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    r->a[3] = 0x027E40u;
    int16_t speed = (int16_t)rd16(m, r->a[0] + 0x42u);
    uint32_t index = (uint32_t)((int32_t)(int16_t)r->d[0] * 2);
    r->d[1] = (uint32_t)(speed * (int16_t)rd16(m, r->a[3] + 0x80u + index));
    r->d[2] = (uint32_t)(speed * (int16_t)rd16(m, r->a[3] + index));
    Obj_MoveByAngle(m, r->a[0], (uint16_t)r->d[0]);
    r->a[7] += 4u;
}

/* ROM 0x11B288, 0x11B2CA and 0x11B262. Register-level camera helpers
 * based on the versions in fn_129338.c, with the nested call frames. */
static inline void boss_camera_step(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    push_long(m, r, pc + 6u);
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, globals + 0x30D2u)));
    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, globals + 0x30D6u)));
    boss_angle(m, r, 0x11B29Eu);
    uint8_t heading = rd8(m, obj + 0x20u);
    int8_t turn = (int8_t)((uint8_t)r->d[0] - heading);
    if (turn > 2)
        turn = 2;
    else if (turn < -2)
        turn = -2;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | (uint8_t)turn;
    wr8(m, obj + 0x20u, (uint8_t)(heading + turn));
    r->a[7] += 4u;

    push_long(m, r, pc + 12u);
    int8_t target = (int8_t)rd8(m, obj + 0x54u);
    int8_t current = (int8_t)rd8(m, obj + 0x56u);
    if (target > current)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) + 0x20u));
    else if (target < current)
        wr16(m, obj + 0x56u, (uint16_t)(rd16(m, obj + 0x56u) - 0x10u));
    uint16_t speed = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x56u);
    r->d[0] = set_word(r->d[0], speed);
    wr16(m, obj + 0x42u, speed);
    r->a[7] += 4u;

    push_long(m, r, pc + 18u);
    wr32(m, obj + 0x14u, rd32(m, globals + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, globals + 0x30D6u));
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x20u) >> 8);
    boss_move(m, r, 0x11B27Au);
    wr32(m, globals + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, globals + 0x30D6u, rd32(m, obj + 0x18u));
    r->a[7] += 4u;
}

/* ROM 0x129724 through 0x129766 and 0x129D46 through 0x129D7E.
 * The entries differ only in the number of sound calls and the next state. */
static inline void boss_camera_change_phase(const f3mem_t *m, f3regs_t *r,
                                             int from_middle_phase)
{
    uint32_t globals = r->a[5];
    uint32_t obj = r->a[0];
    uint32_t next = 0x129768u;

    if (!from_middle_phase)
        FnAuto_PlayHooked(m, r, 0x13u, 0x12972Eu, 0x129734u);
    if (from_middle_phase || (int16_t)rd16(m, globals + 0x30DAu) < 0x1F4) {
        FnAuto_PlayHooked(m, r, 0x13u, 0x129D50u, 0x129D56u);
        next = 0x129D80u;
    }

    wr32(m, globals + 0x30EEu, 0);
    wr8(m, obj + 0x54u, 6);
    wr16(m, obj + 0x50u, 0xFAu);
    wr16(m, obj + 0x52u, 0x78u);
    wr16(m, obj + 6u, 0x3Cu);
    wr32(m, obj, next);
}

#endif
