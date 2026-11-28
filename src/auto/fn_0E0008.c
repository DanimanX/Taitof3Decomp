#include "fn_auto.h"
/* ROM 0x0E0008: seed the movement record and switch to the next object state. */
#include "fn_call_frames.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E0008(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = 0x0DC84Eu;                                 /* 0x0E000A + signext(0xC844) */
    push_long(m, r, 0x0E0012u);
    r->a[2] = Obj_SeedFromParams(m, obj, r->a[1]);         /* 0x0CEC4C */
    uint16_t index = rd8(m, obj + 5u);
    if (rd8(m, obj + 4u) != 5u)
        index += 0x20u;                                  /* 0x0CECA2 */
    r->d[0] = set_word(r->d[0], index);
    r->a[1] += 0x34u;                                     /* 0x0CECDE reads (A1) */
    r->a[7] += 4u;

    wr8(m, obj + 0x23u, 7u);                             /* 0x0E0012 */
    wr32(m, obj + 0x54u, UINT32_MAX);
    wr8(m, obj + 6u, 1u);
    wr32(m, obj, 0x0E002Eu);
    r->a[7] += 4u;
    return 0;
}
