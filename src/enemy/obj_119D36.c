#include "obj_119D36.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x119D36 */
int Obj_119D36_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x0Cu));
    wr32(m, obj + 0x0Cu, rd32(m, obj + 0x0Cu) + 0x200u);
    Obj_Animate(m, obj, 0x00119D94u);        /* lea (0x4c,PC) */
    Obj_DrawSprite(m, obj);
    if ((int16_t)rd16(m, obj + 0x14u) < 0x140) {
        wr32(m, obj + 0x00u, 0x00119D36u);   /* 0x119D2E */
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
