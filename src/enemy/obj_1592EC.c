#include "obj_1592EC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x1592EC: clear pf0b/line_ram, zero A5+0x422C pair, and free object. */
int Obj_1592EC_Tick(const f3mem_t *m, uint32_t obj)
{
    Gfx_ClearPf0b(m);
    Obj_16AB26(m, F3_A5 + 0x422Cu, 0u, 0u);
    Obj_Free(m, obj);
    return 0;
}
