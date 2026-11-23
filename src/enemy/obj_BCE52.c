#include "obj_BCE52.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0xBCE52 */
int Obj_BCE52_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BCE52: animate from the fixed table 0x0BCF92 */
    Obj_AnimFrameStep(m, obj, 0x000BCF92u);

    /* 0x0BCE5E: draw 20 shrunk copies (A1 read below with a large A5 offset */
    wr16(m, obj + 0x5Cu, 0x0013u);
    for (;;) {
        uint16_t d0w = (uint16_t)(rd16(m, obj + 0x5Cu) << 4);
        wr16(m, obj + 0x14u, d0w);
        Obj_DrawSprite(m, obj);
        wr16(m, obj + 0x5Cu, (uint16_t)(rd16(m, obj + 0x5Cu) - 1u));
        if ((int16_t)rd16(m, obj + 0x5Cu) < 0)
            break;
    }

    /* 0x0BCE7A */
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BCE4A */
        wr32(m, obj, 0x000BCE52u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
