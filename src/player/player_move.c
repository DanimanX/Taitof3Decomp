/* Darius Gaiden (Taito F3) — player ship movement.
 * Decompiled from ROM 0x0C691A Player_MoveAndBounds and helpers; operates on a
 * big-endian Work RAM image so it can be checked against MAME captures. */
#include "player_move.h"
#include "../kernel/rng.h"

#define RNG_SEED_ADDR        0x406BAAu
#define PLAYER_CTRL_FLAGS    (F3_A5 + 0x1D36) /* bit3 no joystick, bit4 shake */
#define TERRAIN_COLL_OFF     (F3_A5 + 0x1D1C)
#define TERRAIN_SCR_X        (F3_A5 + 0x0918)
#define TERRAIN_SCR_Y        (F3_A5 + 0x091A)
#define TERRAIN_BITMAP       0x40891Cu
#define STAGE_SCROLL_VX      (F3_A5 + 0x4224)
#define STAGE_SCROLL_VY      (F3_A5 + 0x4228)
#define JOY_TO_ANGLE_TBL     0x0C9462u        /* 16 x s16 */
#define SIN_TBL              0x027E40u        /* s16, cos = SIN_TBL + 0x80 */

/* ROM 0x0C6632: 1 if the terrain bitmap has a solid cell at screen (x,y). */
int Terrain_TestPoint(const f3mem_t *m, int16_t x, int16_t y)
{
    if (rd8(m, TERRAIN_COLL_OFF))
        return 0;
    uint16_t cx = (uint16_t)((uint16_t)(x + (int16_t)rd16(m, TERRAIN_SCR_X)) >> 3) & 0x3FF;
    uint16_t cy = (uint16_t)((uint16_t)(y + (int16_t)rd16(m, TERRAIN_SCR_Y)) >> 3) & 0x3F;
    uint32_t bit = (uint32_t)cy * 0x280u + cx;              /* bftst {bit:1}, MSB first */
    return (rd8(m, TERRAIN_BITMAP + (bit >> 3)) >> (7 - (bit & 7))) & 1;
}

/* ROM 0x0CBE00 */
void Obj_SubScrollVel(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + PS_POS_X, rd32(m, obj + PS_POS_X) - rd32(m, STAGE_SCROLL_VX));
    wr32(m, obj + PS_POS_Y, rd32(m, obj + PS_POS_Y) - rd32(m, STAGE_SCROLL_VY));
}

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle] (muls.w, 16.16 result). */
void Obj_MoveByAngle(const f3mem_t *m, uint32_t obj, uint16_t angle)
{
    int16_t speed = (int16_t)rd16(m, obj + PS_SPEED);
    int16_t c = (int16_t)rd16(m, SIN_TBL + 0x80 + 2u * angle);
    int16_t s = (int16_t)rd16(m, SIN_TBL + 2u * angle);
    wr32(m, obj + PS_POS_X, rd32(m, obj + PS_POS_X) + (uint32_t)((int32_t)speed * c));
    wr32(m, obj + PS_POS_Y, rd32(m, obj + PS_POS_Y) + (uint32_t)((int32_t)speed * s));
}

static uint16_t rng_next_w(const f3mem_t *m) /* Rng_Next on the RAM image; D0.w */
{
    uint32_t seed = rng_step(rd32(m, RNG_SEED_ADDR));
    wr32(m, RNG_SEED_ADDR, seed);
    return (uint16_t)(seed >> 16);
}

static void clamp_hi(const f3mem_t *m, uint32_t a, int16_t lo, int16_t hi)
{
    int16_t v = (int16_t)rd16(m, a); /* only the integer (high) word is touched */
    if (v < lo) wr16(m, a, (uint16_t)lo);
    if ((int16_t)rd16(m, a) > hi) wr16(m, a, (uint16_t)hi);
}

/* ROM 0x0C691A */
void Player_MoveAndBounds(const f3mem_t *m, uint32_t p)
{
    if (rd8(m, p + PS_FLAGS) & 0x01)
        return;
    int16_t old_y = (int16_t)rd16(m, p + PS_POS_Y);
    uint8_t ctrl = rd8(m, PLAYER_CTRL_FLAGS);

    if (!(ctrl & 0x08)) {
        uint8_t joy = rd8(m, rd32(m, p + PS_IO_CH_PTR)) & 0x0F;
        int16_t angle = (int16_t)rd16(m, JOY_TO_ANGLE_TBL + 2u * joy);
        wr16(m, p + PS_ANGLE, (uint16_t)angle);
        if (angle >= 0)
            Obj_MoveByAngle(m, p, (uint16_t)angle);
    }
    if (ctrl & 0x10) { /* screen shake */
        wr32(m, p + PS_POS_X, rd32(m, p + PS_POS_X) + ((uint32_t)(int32_t)(int16_t)rng_next_w(m) << 1));
        wr32(m, p + PS_POS_Y, rd32(m, p + PS_POS_Y) + ((uint32_t)(int32_t)(int16_t)rng_next_w(m) << 1));
    }
    if ((int16_t)rd16(m, p + PS_INVULN) <= 0 &&
        Terrain_TestPoint(m, (int16_t)(rd16(m, p + PS_POS_X) + 0x14),
                             (int16_t)(rd16(m, p + PS_POS_Y) + 0x0E))) {
        wr32(m, p + PS_HP, rd32(m, p + PS_HP) - 0x10000u);
        if ((int16_t)rd16(m, p + PS_ARM_LEVEL) < 10) {
            wr16(m, p + PS_SHIELD, 0);
        } else {
            Obj_SubScrollVel(m, p);
            uint16_t angle = rd16(m, p + PS_ANGLE);
            if (!(angle & 0x8000)) { /* recoil: addi.b #-0x80 on the low byte only */
                angle = (uint16_t)((angle & 0xFF00) | (uint8_t)(angle - 0x80));
                wr16(m, p + PS_SPEED, 9);
                Obj_MoveByAngle(m, p, angle);
                wr16(m, p + PS_SPEED, 12);
            }
        }
    }
    clamp_hi(m, p + PS_POS_X, -8, 0x118);
    clamp_hi(m, p + PS_POS_Y, 0x0E, 0xD2);

    /* banking: compare integer Y words, step 16.16 tilt by 1/8 up to +-2 */
    int16_t new_y = (int16_t)rd16(m, p + PS_POS_Y);
    int16_t tilt_hi = (int16_t)rd16(m, p + PS_BANK);
    int dir = 0;                      /* +1: add, -1: sub */
    if (old_y > new_y) dir = 1;       /* moved up */
    else if (old_y < new_y) dir = -1; /* moved down */
    else if (tilt_hi < 0) dir = 1;    /* idle: return to level */
    else if (tilt_hi > 0) dir = -1;
    if (dir > 0 && tilt_hi < 2)
        wr32(m, p + PS_BANK, rd32(m, p + PS_BANK) + 0x2000u);
    else if (dir < 0 && tilt_hi > -2)
        wr32(m, p + PS_BANK, rd32(m, p + PS_BANK) - 0x2000u);
}
