#include "obj_12E19A.h"

/* ROM 0x12E19A */
int Obj_12E19A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12E19A */
    wr8(m, F3_A5 + 0x32AEu, 0xFFu);         /* st */
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);         /* st */
    wr16(m, F3_A5 + 0x32A8u, 0u);
    wr16(m, F3_A5 + 0x32A4u, 0u);
    wr16(m, F3_A5 + 0x32A6u, 0u);
    wr16(m, obj + 0x06u, 0x0002u);

    /* 0x12E1B6 */
    wr32(m, obj, 0x0012E1BEu);
    return 0;
}
