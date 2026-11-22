#include "obj_191356.h"
/* ROM 0x191356 */
int Obj_191356_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x24u, 0x00010000u);  /* 0x191356 */
    wr32(m, obj + 0x28u, 0x00010000u);  /* 0x19135E */
    wr8(m, obj + 0x34u, 0x0Au);         /* 0x191366 */
    /* 0x19136C: falls through to 0x191372 */
    wr32(m, obj + 0x00u, 0x00191374u);
    return 0;
}
