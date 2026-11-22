#include "obj_11B288.h"

/* ROM 0x0011B288: turn +0x20 toward the leader by at most 2/frame. The body
 * is ObjHelper_11B288 (emitted once in obj_11B262.c, see bundle_11B288.s /
 * the copy in src/enemy/obj_126B10.c). */
int Obj_11B288_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);
    return 0;
}
