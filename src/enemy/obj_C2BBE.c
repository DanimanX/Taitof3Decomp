#include "obj_C2BBE.h"
#include "../video/sprite.h"

/* ROM 0x0C2BBE */
int Obj_C2BBE_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x4Cu));
    wr32(m, obj + 0x00u, 0x000C2BBEu);
    return 0;
}
