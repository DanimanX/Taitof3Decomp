#include "obj_D853C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D853C */
int Obj_D853C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x3Au, 0u);
    wr32(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x28u, 1u);
    wr16(m, obj + 0x30u, 2u);
    wr16(m, obj + 0x32u, 0x0Du);
    wr8(m, obj + 0x64u, rd8(m, rd32(m, obj + 0x08u) + 0x64u));
    wr16(m, obj + 0x06u, 0x32u);
    wr32(m, obj, 0xD8574u);
    return 0;
}
