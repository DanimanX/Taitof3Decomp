#include "obj_116078.h"
#include "obj_112C3C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x116078 */
int Obj_116078_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    uint8_t fl = rd8(m, p + 0x50u);
    if ((fl & 0x10u) != 0u || ((fl & 0x08u) == 0u && (fl & 0x01u) != 0u)) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_Animate(m, obj, 0x0011624Au);
    Obj_113400_FollowParent(m, obj);
    Obj_DrawSprite(m, obj);
    int16_t c = (int16_t)(rd16(m, obj + 0x06u) - 1);
    wr16(m, obj + 0x06u, (uint16_t)c);
    if (c > 0) {
        wr32(m, obj, 0x00116078u);
        return 0;
    }
    wr16(m, obj + 0x06u, 0x0050u);
    wr32(m, obj, 0x001160C4u);
    return 0;
}
