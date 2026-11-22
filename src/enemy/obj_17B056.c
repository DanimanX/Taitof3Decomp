#include "obj_17B056.h"
#include "../objects/obj_pool.h"

/* ROM 0x17B056: hand the object to the palette cycler 0x17B05E. */
int Obj_17B056_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x17B05Eu);
    return 0;
}
