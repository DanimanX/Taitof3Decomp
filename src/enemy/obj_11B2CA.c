#include "obj_11B2CA.h"

/* ROM 0x0011B2CA: ease +0x56 toward the target byte at +0x54 and mirror it
 * into +0x42. The body is ObjHelper_11B2CA (emitted once in obj_11B262.c,
 * see the copy in src/enemy/obj_126B10.c). */
int Obj_11B2CA_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B2CA(m, obj);
    return 0;
}
