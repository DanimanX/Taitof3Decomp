#include "fn_auto.h"
/* ROM 0x0E343C: seed a trail object and arm its next state. */
#include "fn_call_frames.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E343C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = 0x0E343Eu - 0x6BF0u;                             /* 0x0E343C LEA */
    push_long(m, r, 0x0E3446u);                                  /* 0x0E3440 JSR */
    r->d[0] = set_word(r->d[0], (uint16_t)(rd8(m, obj + 5u)
                               + (rd8(m, obj + 4u) == 5u ? 0u : 0x20u)));
    r->a[2] = Obj_SeedFromParams(m, obj, r->a[1]);                 /* 0x0CEC4C */
    r->a[1] += 0x34u;                                            /* 0x0CECDE (A1) */
    wr8(m, obj + 0x23u, 7);
    wr32(m, obj + 0x54u, 0xFFFFFFFFu);
    wr8(m, obj + 6u, 1);
    wr32(m, obj, 0x0E3462u);
    r->a[7] += 8u;                                               /* both RTS */
    return 0;
}
