#include "obj_119360.h"
#include "../video/sprite.h"

/* ROM 0x119616: draw a string with the shared sprite object 0x41B4EE.
 * Returns the end position as (x << 16) | y. */
static uint32_t sub_119616(const f3mem_t *m, uint32_t str, uint16_t x, uint16_t y)
{
    uint32_t o = 0x0041B4EEu;
    uint32_t a2 = F3_A5 + 0x30CAu;
    wr32(m, o + 0x10u, a2);
    wr16(m, o + 0x1Cu, 0u);
    wr16(m, o + 0x1Eu, 0u);
    wr16(m, o + 0x14u, x);
    wr16(m, o + 0x18u, y);
    wr32(m, a2, 0x00010001u);
    a2 += 4u;
    for (;;) {
        uint8_t c = rd8(m, str++);
        if (c == 0u)
            break;
        uint32_t idx = (uint8_t)(c - 0x20u);
        wr32(m, a2, rd32(m, 0x00059304u + idx * 4u));
        Obj_DrawSprite(m, o);
        wr16(m, o + 0x14u, (uint16_t)(rd16(m, o + 0x14u) + 0x10u));
    }
    return ((uint32_t)rd16(m, o + 0x14u) << 16) | rd16(m, o + 0x18u);
}

/* ROM 0x119360 */
int Obj_119360_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = sub_119616(m, 0x0011953Bu, 0x0030u, 0x0058u);    /* lea (0x1d9,PC) */
    uint32_t a1 = 0x0013D5C8u + (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x0Eu) * 2);
    p = sub_119616(m, a1, (uint16_t)(p >> 16), (uint16_t)p);
    sub_119616(m, 0x00119541u, (uint16_t)(p >> 16), (uint16_t)p);   /* lea (0x1b7,PC) */
    wr32(m, obj + 0x00u, 0x00119360u);       /* 0x119358 */
    return 0;
}
