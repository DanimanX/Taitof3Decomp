#include "obj_117BDE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x117BDE */
int Obj_117BDE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x00010000u);

    Obj_DrawSprite(m, obj);

    uint16_t t = (uint16_t)(rd16(m, obj + 0x44u) - 1u);
    wr16(m, obj + 0x44u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x00117BDEu);
        return 0;
    }

    Obj_Free(m, obj);
    return 0;
}
