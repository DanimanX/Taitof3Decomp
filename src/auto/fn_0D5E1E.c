#include "fn_auto.h"
/* ROM 0x0D5E1E: Step the looping animation at 0x0D5C1E and apply its sprite flip bits. */
#include "fn_call_frames.h"
#include "../objects/obj_common.h"

int Fn_0D5E1E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = 0x0D5C1Eu;
    push_long(m, r, 0x0D5E28u);
    Obj_Animate(m, obj, r->a[1]); /* 0x0D5E22 -> 0x0CBD52 */
    r->a[7] += 4u;
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x3Au));

    uint16_t sprite_high = rd16(m, obj + 0x10u);
    r->d[1] = sprite_high >> 14; /* 0x0D5E28 BFEXTU {0:2} */
    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x1Cu) ^ (uint8_t)r->d[1]); /* 0x0D5E2E */
    wr16(m, obj + 0x10u, sprite_high & 0x3FFFu); /* 0x0D5E32 ANDI.W */
    r->a[7] += 4u; /* 0x0D5E38 RTS */
    return 0;
}
