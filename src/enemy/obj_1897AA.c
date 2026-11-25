#include "obj_1897AA.h"

/* ROM 0x1897AA: arm 10-frame delay in +0x06 and transition to 0x1897BA */
int Obj_1897AA_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0x000Au);
    wr32(m, obj + 0x00u, 0x001897BAu);
    return 0;
}
