#include "obj_D823E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D823E */
int Obj_D823E_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x28u, 0u);
    Obj_Free(m, obj);
    return 0;
}
