#include "obj_116F7A.h"

/* ROM 0x116F7A: set entry-point pointer */
int Obj_116F7A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00116F82u);
    return 0;
}
