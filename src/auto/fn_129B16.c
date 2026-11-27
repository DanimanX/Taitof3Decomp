#include "fn_auto.h"
/* ROM 0x129B16: Steer and move the boss camera, then advance its timed script. */
#include "../enemy/obj_11B262.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

/* ROM 0x11B288: steer toward the target, clamping the signed byte delta to ±2. */
static void sub_11B288(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];
    uint32_t sp = r->a[7] - 4u;
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, globals + 0x30D2u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, globals + 0x30D6u));
    r->d[1] = (r->d[1] & 0xFFFF0000u) | dy;

    /* 0x11B298 calls 0x006422, which saves D1-D3/A0 and can exchange D0/D1. */
    wr32(m, sp, 0x11B29Eu);
    wr32(m, sp - 4u, obj);
    wr32(m, sp - 8u, r->d[3]);
    wr32(m, sp - 12u, r->d[2]);
    wr32(m, sp - 16u, r->d[1]);
    uint32_t angle_hi = ((dx ^ dy) & 0x8000u) ? r->d[1] : r->d[0];
    uint16_t angle = Obj_AtanByte(m, (int16_t)dx, (int16_t)dy);
    uint8_t heading = rd8(m, obj + 0x20u);
    int8_t delta = (int8_t)(angle - heading);
    if (delta > 2)
        delta = 2;
    else if (delta < -2)
        delta = -2;
    wr8(m, obj + 0x20u, (uint8_t)(heading + delta));
    r->d[0] = (angle_hi & 0xFFFF0000u) | (uint8_t)delta;
}

/* ROM 0x11B2CA: ease the speed accumulator toward the target byte. */
static void sub_11B2CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    ObjHelper_11B2CA(m, obj);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, obj + 0x42u);
}

/* ROM 0x11B262: move the camera through the object's position fields. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];
    wr32(m, obj + 0x14u, rd32(m, globals + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, globals + 0x30D6u));
    uint16_t angle = rd8(m, obj + 0x20u);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | angle;

    wr32(m, r->a[7] - 4u, 0x11B27Au);
    r->a[3] = 0x27E40u;
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    r->d[1] = (uint32_t)((int32_t)speed * (int16_t)rd16(m, r->a[3] + 0x80u + 2u * angle));
    r->d[2] = (uint32_t)((int32_t)speed * (int16_t)rd16(m, r->a[3] + 2u * angle));
    Obj_MoveByAngle(m, obj, angle);
    wr32(m, globals + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, globals + 0x30D6u, rd32(m, obj + 0x18u));
}

int Fn_129B16(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x129B1Cu);
    sub_11B288(m, r);
    wr32(m, r->a[7], 0x129B22u);
    sub_11B2CA(m, r);
    wr32(m, r->a[7], 0x129B28u);
    sub_11B262(m, r);
    r->a[7] += 4u;

    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u)); /* 0x129B28 */
    if ((int16_t)timer > 1) { /* 0x129B2C branches to 0x129B0E. */
        wr32(m, obj, 0x129B16u);
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) | 0x80u)); /* 0x129B2E */
    wr8(m, globals + 0x30EEu, (uint8_t)(rd8(m, globals + 0x30EEu) | 0x10u)); /* 0x129B34 */
    wr16(m, obj + 0x6u, 0x12Cu); /* 0x129B3A */
    /* 0x129B40 BEQ is unreachable after loading the nonzero timer. */
    wr32(m, obj, 0x129B4Au); /* 0x129B42 */
    r->a[7] += 4u;
    return 0;
}
