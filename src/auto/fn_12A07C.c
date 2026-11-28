#include "fn_auto.h"
/* ROM 0x12A07C: boss camera tick; when its timer runs out set A5+0x30EE bit 8 and arm 0x12A0AA. */
#include "round8_camera_regs.h"

int Fn_12A07C(const f3mem_t *m, f3regs_t *r)
{
    camera_chain_step(m, r, 0x12A07Cu, 0x12A0AAu, 0x10u);
    r->a[7] += 4u;                       /* rts */
    return 0;
}
