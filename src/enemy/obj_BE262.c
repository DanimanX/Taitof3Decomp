#include "obj_BE262.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BE262: type one character per frame into text RAM (0x61C000).
 * Text RAM is not modelled: only the Work RAM effects are reproduced. */
int Obj_BE262_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint8_t c = rd8(m, a1);
    wr32(m, obj + 0x44u, a1 + 1u);
    if (c == 0x00u) {                      /* newline */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 1u));
        wr16(m, obj + 0x14u, 0x0001u);
    } else if (c == 0xFFu) {               /* end */
        Obj_Free(m, obj);
        return 0;
    } else {
        /* 0x0BE286: text RAM write {0x0E, c} at 0x61C000 + y*128 + x*2 skipped */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
    }
    wr32(m, obj + 0x00u, 0x000BE262u);
    return 0;
}
