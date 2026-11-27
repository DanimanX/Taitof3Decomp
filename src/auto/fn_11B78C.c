#include "fn_auto.h"
/* ROM 0x11B78C: verified on a capture from the Y04 re-survey states: Initialize a child at its parent's position and seed its delay and heading. */
#include "fn_call_frames.h"
#include "../objects/obj_common.h"

/* 0x0CCC08: unpack the spawn argument and inherit the parent's position. */
static void inherit_position(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t arg = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, rd16(m, obj + 0x50u) + (int8_t)(arg >> 24));
    wr16(m, obj + 0x52u, rd16(m, obj + 0x52u) + (int8_t)(arg >> 16));
    r->d[1] = (arg >> 8) & 0xFFu;
    wr16(m, obj + 0x20u, (uint16_t)r->d[1]);
    r->a[1] = rd32(m, obj + 8u);
    wr32(m, obj + 0x14u, rd32(m, r->a[1] + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, r->a[1] + 0x18u));
    wr16(m, obj + 0x34u, 0);
    wr32(m, obj + 0x28u, 0x10000u);
    wr16(m, obj + 0x14u, rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u));
    r->d[0] = set_word(arg, rd16(m, obj + 0x52u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x18u) + (uint16_t)r->d[0]);
    wr16(m, obj + 0x3Au, 0);
    wr16(m, obj + 0x3Eu, 0);
}

int Fn_11B78C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_long(m, r, 0x11B792u);
    Obj_ResetFields(m, obj); /* 0x0CBD86 */
    r->a[7] += 4u;
    wr16(m, obj + 0x2Cu, 4);
    wr16(m, obj + 0x2Eu, 0x1B);
    wr16(m, obj + 0x30u, 4);
    wr16(m, obj + 0x32u, 0x1B);
    wr16(m, obj + 0x50u, 0xFFF0u);
    wr16(m, obj + 0x52u, 0xFFF0u);
    push_long(m, r, 0x11B7BCu);
    inherit_position(m, r);
    r->a[7] += 4u;
    wr32(m, obj + 0x28u, 0x27100000u);
    wr32(m, obj + 0x24u, 0x10000u);
    wr16(m, obj + 0x42u, 0);

    /* 0x11B7D0 -> 0x11B884. The clear above fixes the 0x11B89A branch. */
    push_long(m, r, 0x11B88Au);
    r->d[0] = Rng_NextMem(m);
    r->a[7] += 4u;
    r->d[0] = set_word(r->d[0], (r->d[0] & 0x1Fu) + 0xEu);
    wr16(m, obj + 6u, (uint16_t)r->d[0]);
    wr16(m, obj + 0x42u, 3);
    push_long(m, r, 0x11B8B2u);
    r->d[0] = Rng_NextMem(m);
    r->a[7] += 4u;
    uint16_t heading = (uint8_t)((r->d[0] & 0x3Fu) - 0x20u);
    heading += rd16(m, obj + 0x20u);
    r->d[0] = set_word(r->d[0], heading);
    wr8(m, obj + 0x20u, (uint8_t)heading);

    wr32(m, obj, 0x11B7DCu); /* 0x11B8C2 -> 0x11B7D4 */
    r->a[7] += 4u;
    return 0;
}
