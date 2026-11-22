#include "obj_13DE92.h"

/* ROM 0x13DE92 */
int Obj_13DE92_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x13DE92: jsr 0x0CBD86 */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);

    wr32(m, obj + 0x10u, 0x000625F4u);
    wr16(m, obj + 0x1Eu, 0xE0E0u);
    wr32(m, obj + 0x00u, 0x0013DEAEu);
    return 0;
}
