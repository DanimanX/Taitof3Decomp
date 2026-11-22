#include "obj_18A4D4.h"
/* ROM 0x18A4D4 */
int Obj_18A4D4_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Eu, 0u);           /* 0x18A4D4 */
    wr16(m, obj + 0x1Cu, 0u);           /* 0x18A4D8 */
    wr32(m, obj + 0x10u, 0x00028740u);  /* 0x18A4DC */
    /* 0x18A4E4: falls through to 0x18A4EC */
    wr32(m, obj + 0x00u, 0x0018A4ECu);
    return 0;
}
