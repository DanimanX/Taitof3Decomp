#include "obj_10DBD8.h"

/* ROM 0x10DBD8: set object pointer entry */
int Obj_10DBD8_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x10DBE0u);
    return 0;
}
