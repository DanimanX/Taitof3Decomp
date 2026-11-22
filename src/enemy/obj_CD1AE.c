#include "obj_CD1AE.h"

/* ROM 0x0CD1AE */
int Obj_CD1AE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x3Au, 0x000CD1B6u);
    wr16(m, obj + 0x1Eu, 0xC0C0u);
    wr16(m, obj + 0x42u, 0x1010u);
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj + 0x10u, 0x00062D98u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x00u, 0x000CCCA2u);
    return 0;
}
