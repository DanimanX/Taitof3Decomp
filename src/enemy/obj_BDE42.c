#include "obj_BDE42.h"

/* ROM 0x0BDE42 */
int Obj_BDE42_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x000BDE42u);
        return 0;
    }
    /* 0x0BDE48: two jsr 0x53B2 (tilemap 0x90A2A -> 0x6133CC / 0x61338C) only
     * write playfield RAM, outside Work RAM. */
    wr16(m, obj + 0x60u, 0x001Fu);
    wr32(m, obj + 0x00u, 0x000BDE92u);
    return 0;
}
