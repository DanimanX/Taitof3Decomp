#include "obj_111492.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0CBD86: clear the shared draw/animation fields */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x111492: seed the draw fields from the spawn argument, then chain on */
int Obj_111492_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x18u, rd32(m, a2 + 0x18u));
    wr32(m, obj + 0x14u, rd32(m, a2 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr32(m, obj + 0x00u, 0x1114B4u);
    return 0;
}
