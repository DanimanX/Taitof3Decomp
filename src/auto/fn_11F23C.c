#include "fn_auto.h"

/* ROM 0x11F23C: UNVERIFIED (no capture): advance the shared position and start the next timed state when the countdown expires. */
#include "../player/player_move.h"

/* ROM 0x11B262: step the shared position, including the 0x0CBE12 call frame. */
static void step_shared_position(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7];
    uint32_t shared = r->a[5] + 0x30D2u;
    wr32(m, sp - 4u, 0x11F242u); /* 0x11F23C */
    wr32(m, obj + PS_POS_X, rd32(m, shared));
    wr32(m, obj + PS_POS_Y, rd32(m, shared + 4u));
    uint16_t angle = rd16(m, obj + 0x20u) >> 8;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | angle;

    wr32(m, sp - 8u, 0x11B27Au); /* 0x11B274, after the position copies */
    r->a[3] = 0x027E40u;
    int32_t speed = (int16_t)rd16(m, obj + PS_SPEED);
    /* Full-format extensions 0x0320 use D0.w*2 with a word base displacement. */
    uint32_t entry = r->a[3] + 2u * angle;
    r->d[1] = (uint32_t)(speed * (int16_t)rd16(m, entry + 0x80u));
    r->d[2] = (uint32_t)(speed * (int16_t)rd16(m, entry));
    Obj_MoveByAngle(m, obj, angle);
    wr32(m, shared, rd32(m, obj + PS_POS_X)); /* 0x11B27A */
    wr32(m, shared + 4u, rd32(m, obj + PS_POS_Y));
}

int Fn_11F23C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    step_shared_position(m, r);
    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u)); /* 0x11F242 */
    /* BGT uses N, V and Z from SUBQ, including the 0x8000 overflow case. */
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x11F23Cu); /* 0x11F234 */
    } else {
        if ((int16_t)rd16(m, r->a[5] + 0x87Eu) >= 0x30)
            wr8(m, r->a[5] + 0x30F2u, 4u); /* 0x11F252 */
        wr16(m, obj + 6u, 0x1Eu);
        wr32(m, obj, 0x11F266u); /* 0x11F25E */
    }
    r->a[7] += 4u;
    return 0;
}
