#include "obj_E0278.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0E0278 */
int Obj_E0278_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t a2 = rd32(m, a1 + 0x0Cu);

    if (rd8(m, a2 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    Obj_DrawSprite(m, a2);
    Obj_DrawSprite(m, a1);
    return 0;
}
