#include "fn_auto.h"
/* ROM 0x0D5880: Step the looping sprite animation at 0x0D568C. */
#include "fn_call_frames.h"
#include "../objects/obj_common.h"

int Fn_0D5880(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = 0x0D568Cu; /* 0x0D5880: 0x0D5882 - 0x1F6 */
    push_long(m, r, 0x0D588Au);
    Obj_Animate(m, obj, r->a[1]); /* 0x0D5884 -> 0x0CBD52 */
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = set_word(r->d[1], rd16(m, obj + 0x3Eu));
    r->a[7] += 4u; /* 0x0CBD84 RTS */
    r->a[7] += 4u; /* 0x0D588A RTS */
    return 0;
}
