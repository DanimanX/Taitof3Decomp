#include "obj_111D18.h"
#include "../objects/obj_pool.h"

/* ROM 0x111D18: bomb burst — bonus pickup plus two side bursts, then waits
 * on 0x111D8E (which frees the object). */
int Obj_111D18_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x58u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x7D0u);

    Obj_Spawn(m, obj, 0x00117B44u, 5u, 2u, 0x000007D0u);

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x74u));

    Obj_Spawn(m, obj, 0x00111D9Cu, 0u, 1u, rd32(m, obj + 0x5Cu));
    Obj_Spawn(m, obj, 0x00111DA6u, 0u, 1u, rd32(m, obj + 0x5Cu));

    wr32(m, obj + 0x00u, 0x00111D8Eu);
    return 0;
}
