#include "obj_BE262.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BE262: type one character per frame into text RAM (0x61C000). */
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
        uint16_t d1w = rd16(m, obj + 0x14u);
        uint16_t d2w = rd16(m, obj + 0x18u);
        uint16_t off = (uint16_t)((uint16_t)(d2w << 7) + (uint16_t)(d1w + d1w));
        uint32_t dst = 0x0061C000u + (uint32_t)(int32_t)(int16_t)off;
        wr8(m, dst, 0x0Eu);
        wr8(m, dst + 1u, c);
        wr16(m, obj + 0x14u, (uint16_t)(d1w + 1u));
    }
    wr32(m, obj + 0x00u, 0x000BE262u);
    return 0;
}
