#include "obj_BE248.h"

/* ROM 0x0BE248: park at (1, 1), latch +0x0C into +0x44, arm 0x0BE262. */
int Obj_BE248_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0x0001u);
    wr16(m, obj + 0x18u, 0x0001u);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x0Cu));
    wr32(m, obj + 0x00u, 0x000BE262u);
    return 0;
}
