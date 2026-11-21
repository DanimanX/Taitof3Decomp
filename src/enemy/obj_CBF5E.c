#include "obj_CBF5E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CBF56 */
static void sub_0CBF56(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x000CBF5Eu);
}

/* ROM 0x0CBF64 */
static void sub_0CBF64(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x000CBF6Cu);
}

/* ROM 0x0CBF5E */
int Obj_CBF5E_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, obj + 0x54u) != 0u) {
        sub_0CBF56(m, obj);
        return 0;
    }
    sub_0CBF64(m, obj);
    return 0;
}
