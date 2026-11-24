#include "obj_17CE88.h"
#include "../objects/obj_pool.h"

/* ROM 0x17CE88: tail of the 0x17CE0A state. The body keeps running until the
 * +0x1E byte reaches 0xE0, at which point the object is released; anything
 * below that falls back into 0x17CE0A, which this entry does not cover. */
int Obj_17CE88_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x1Eu) != 0xE0u) {
        /* 0x17CE8E bne.w -> 0x17CE0A body, outside this entry's window */
        return -1;
    }
    Obj_Free(m, obj); /* 0x17CE94 */
    return 0;
}
