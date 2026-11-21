#include "obj_BDC5C.h"
#include "../objects/obj_common.h"

/* ROM 0x0BDC5C: uploads 13 sprite descriptors to the video MMIO windows
 * (0x61xxxx, 0x62B000) - no Work RAM effect - then hands over to 0x0BDDB8. */
int Obj_BDC5C_Tick(const f3mem_t *m, uint32_t obj)
{
    (void)m;
    wr32(m, obj + 0x00u, 0x000BDDB8u);
    return 0;
}
