#include "fn_auto.h"
/* ROM 0x129EDE: boss camera tick; when its timer runs out set A5+0x30EE bit 8 and arm 0x129F0C. */
#include "round8_camera_regs.h"

int Fn_129EDE(const f3mem_t *m, f3regs_t *r)
{
    camera_chain_step(m, r, 0x129EDEu, 0x129F0Cu, 0x10u);
    r->a[7] += 4u;                       /* rts */
    return 0;
}
