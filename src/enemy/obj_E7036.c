#include "obj_E7036.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0E7036 */
int Obj_E7036_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, a1);
    return 0;
}
