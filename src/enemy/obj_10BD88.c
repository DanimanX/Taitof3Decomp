#include "obj_10BD88.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x10BD88: follow the parent (A0+0x8) when it is alive, else start the
 * death sequence; drift by the object's own +0x50/+0x54 offsets, step the
 * 6-byte script and draw */
int Obj_10BD88_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a2 = rd32(m, obj + 0x08u);                  /* parent */
    if (rd8(m, a2 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);                          /* 0x10BD90 */
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, a2 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr32(m, obj + 0x18u, rd32(m, a2 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x54u)));

    Obj_StepScript(m, obj);                              /* 0x10BDB2: ROM 0x0D0690 */
    Obj_DrawSprite(m, obj);                              /* 0x10BDB8 */
    return 0;
}
