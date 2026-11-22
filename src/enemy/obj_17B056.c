#include "obj_17B056.h"

/* ROM 0x17B056: fn = 0x17B05E */
int Obj_17B056_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x17B05Eu);
    return 0;
}
