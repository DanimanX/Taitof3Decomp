#include "fn_auto.h"
/* ROM 0x11F21C: verified on a capture from the Y04 re-survey states: advance the shared coordinates by angle, then change state when the timer expires. */
#include "../player/player_move.h"

/* 0x11B262: stage the shared coordinates in the object for 0x0CBE12. */
static void advance_coords(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t x = r->a[5] + 0x30D2u;
    uint32_t y = r->a[5] + 0x30D6u;
    wr32(m, obj + 0x14u, rd32(m, x));
    wr32(m, obj + 0x18u, rd32(m, y));
    uint16_t angle = rd16(m, obj + 0x20u) >> 8;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | angle;
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x11B27Au); /* 0x11B274 JSR */
    /* 0x0CBE1E and 0x0CBE28 use full-format D0.w*2 with base
     * displacements 0x80 and 0, respectively. */
    r->a[3] = 0x027E40u;
    uint32_t index = r->a[3] + 2u * angle;
    int32_t speed = (int16_t)rd16(m, obj + 0x42u);
    r->d[1] = (uint32_t)(speed * (int16_t)rd16(m, index + 0x80u));
    r->d[2] = (uint32_t)(speed * (int16_t)rd16(m, index));
    Obj_MoveByAngle(m, obj, angle);
    r->a[7] += 4u; /* 0x0CBE32 RTS */
    wr32(m, x, rd32(m, obj + 0x14u));
    wr32(m, y, rd32(m, obj + 0x18u));
    r->a[7] += 4u; /* 0x11B286 RTS */
}

int Fn_11F21C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x11F222u);
    advance_coords(m, r);

    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x11F21Cu); /* 0x11F214 */
    } else {
        wr16(m, obj + 0x20u, 0xC000u); /* 0x11F228 */
        wr16(m, obj + 6u, 4u); /* 0x11F22E */
        wr32(m, obj, 0x11F23Cu); /* 0x11F234 */
    }
    r->a[7] += 4u;
    return 0;
}
