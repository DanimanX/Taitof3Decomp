#include "obj_C7480.h"
#include "obj_1194C2.h"                                 /* Text_DrawString */

/* ROM 0x0C7480: print the string at 0x0C95D3 twice in text RAM, at
 * 0x61C000 + word table 0x0C915A[+0x22] and one row pair (0x80) below it,
 * then arm 0x0C74CC. */
int Obj_C7480_Tick(const f3mem_t *m, uint32_t obj)
{
    /* lea (0x1CCE,PC,D0.w*2),A2 -- full-format ext at 0x0C748C */
    uint16_t idx = rd16(m, obj + 0x22u);
    uint32_t a1 = 0x0061C000u
        + (uint32_t)(int32_t)(int16_t)rd16(m, 0x000C915Au + (uint32_t)(int32_t)(int16_t)idx * 2u);
    (void)Text_DrawString(m, 0x000C95D3u, a1, 0u);
    (void)Text_DrawString(m, 0x000C95D3u, a1 + 0x80u, 0u);
    wr32(m, obj + 0x00u, 0x000C74CCu);
    return 0;
}
