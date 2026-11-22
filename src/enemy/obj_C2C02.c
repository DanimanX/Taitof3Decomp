#include "obj_C2C02.h"
#include "../video/sprite.h"

/* ROM 0x0C2C02 */
int Obj_C2C02_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x4Cu));
    wr32(m, obj + 0x00u, 0x000C2C02u);
    return 0;
}
