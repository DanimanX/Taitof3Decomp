#include "obj_13DF4C.h"
#include "../objects/obj_pool.h"

/* ROM 0x13DF4C */
int Obj_13DF4C_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x32FEu) != 0u) {
        wr32(m, obj + 0x00u, 0x0013DF5Au);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x0013DF4Cu);      /* 0x13DF44 */
    return 0;
}
