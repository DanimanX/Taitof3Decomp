#include "obj_1134B2.h"

/* ROM 0x1134B2 */
int Obj_1134B2_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x001134BAu);
    return 0;
}
