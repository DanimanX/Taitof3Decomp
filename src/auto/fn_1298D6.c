#include "fn_auto.h"
/* ROM 0x1298D6: Steer and move the boss camera, then advance its timed script. */
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

int Fn_1298D6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];

    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x1298DCu);
    sub_11B288(m, r);
    wr32(m, r->a[7], 0x1298E2u);
    sub_11B2CA(m, r);
    wr32(m, r->a[7], 0x1298E8u);
    sub_11B262(m, r);
    r->a[7] += 4u;

    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u)); /* 0x1298E8 */
    if ((int16_t)timer > 1) { /* 0x1298EC branches to 0x1298CE. */
        wr32(m, obj, 0x1298D6u);
        r->a[7] += 4u;
        return 0;
    }

    if ((int16_t)rd16(m, globals + 0x30DAu) < 0x01F4) { /* 0x1298EE..0x1298F4 */
        /* 0x129D46 changes to the final camera phase. */
        FnAuto_PlayHooked(m, r, 0x13u, 0x129D50u, 0x129D56u);
        wr32(m, globals + 0x30EEu, 0u);
        wr8(m, obj + 0x54u, 6u);
        wr16(m, obj + 0x50u, 0xFAu);
        wr16(m, obj + 0x52u, 0x78u);
        wr16(m, obj + 6u, 0x3Cu);
        wr32(m, obj, 0x129D80u);
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x54u, 0x2u); /* 0x1298F8 */
    wr16(m, obj + 0x50u, 0xF0u); /* 0x1298FE */
    wr16(m, obj + 0x52u, 0x6Eu); /* 0x129904 */
    wr32(m, globals + 0x30E2u, rd32(m, globals + 0x1D28u)); /* 0x12990A */
    wr8(m, globals + 0x30F2u, 0x8u); /* 0x129910 */
    wr16(m, obj + 0x6u, 0x3Cu); /* 0x129916 */
    /* 0x12991C BEQ is unreachable after loading the nonzero timer. */
    wr32(m, obj, 0x129926u); /* 0x12991E */
    r->a[7] += 4u;
    return 0;
}
