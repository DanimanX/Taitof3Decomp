#include "obj_1134BA.h"

/* ROM 0x1134BA: wait for the parent's state flags */
int Obj_1134BA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint8_t f = rd8(m, rd32(m, obj + 0x08u) + 0x50u);
    uint32_t fn;
    if (f & 0x20u)
        fn = 0x00113644u;           /* 0x11363C */
    else if (f & 0x18u)
        fn = 0x001134E6u;           /* 0x1134DE */
    else
        fn = 0x001134BAu;           /* 0x1134B2 */
    wr32(m, obj + 0x00u, fn);
    return 0;
}
