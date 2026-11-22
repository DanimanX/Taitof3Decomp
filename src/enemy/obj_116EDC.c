#include "obj_116EDC.h"

/* ROM 0x116EDC: set obj->func_ptr to Obj_116EE4 and return */
int Obj_116EDC_Tick(const f3mem_t *m, uint32_t obj)
{
    (void)m;
    wr32(m, obj + 0x00u, 0x0116EE4u);
    return 0;
}
