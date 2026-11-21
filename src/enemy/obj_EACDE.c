#include "obj_EACDE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x000EACDE */
int Obj_EACDE_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    return 0;
}
