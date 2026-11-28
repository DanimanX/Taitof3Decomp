#include "fn_auto.h"
/* ROM 0x12A04E: boss camera tick; when its timer runs out set A5+0x30EE bit 8 and arm 0x12A07C. */
#include "round8_camera_regs.h"

int Fn_12A04E(const f3mem_t *m, f3regs_t *r)
{
    camera_chain_step(m, r, 0x12A04Eu, 0x12A07Cu, 0x78u);
    r->a[7] += 4u;                       /* rts */
    return 0;
}
