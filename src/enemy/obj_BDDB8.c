#include "obj_BDDB8.h"
#include "../objects/obj_common.h"

/* ROM 0x0BDDB8: sound/visual subsystem reset; only MMIO (0x61xxxx/0x62xxxx,
 * outside Work RAM) plus A5 -0x5D90/-0x5D68 (0x402270.., outside RAM) are
 * touched, so the Work RAM image is unchanged. */
int Obj_BDDB8_Tick(const f3mem_t *m, uint32_t obj)
{
    (void)m;
    wr32(m, obj + 0x00u, 0x000BDDE2u);
    return 0;
}
