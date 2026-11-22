#include "obj_119270.h"

/* ROM 0x005666: write a NUL-terminated string as {attr, char} byte pairs;
 * attr = colour * 2. Returns the end address. */
static uint32_t sub_005666(const f3mem_t *m, uint32_t str, uint32_t dst, uint16_t colour)
{
    uint8_t attr = (uint8_t)(colour + colour);
    for (;;) {
        uint8_t c = rd8(m, str++);
        if (c == 0u)
            return dst;
        wr8(m, dst++, attr);
        wr8(m, dst++, c);
    }
}

/* ROM 0x1195D0: draw the +0x3A text entries at +0x44 {col.w, row.w,
 * colour:8 | string:24} */
static void sub_1195D0(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)rd16(m, obj + 0x3Au);
    uint32_t a1 = obj + 0x44u;
    while (--n >= 0) {
        uint16_t col = rd16(m, a1);
        uint16_t row = rd16(m, a1 + 2u);
        uint32_t d0 = rd32(m, a1 + 4u);
        a1 += 8u;
        uint16_t d2 = (uint16_t)(col + col + (uint16_t)(row << 7));
        uint32_t dst = (uint32_t)(int32_t)(int16_t)d2 + 0x0061C000u;
        sub_005666(m, d0 & 0x00FFFFFFu, dst, (uint16_t)(d0 >> 24));
    }
}

/* ROM 0x119590: type the character at +0x22 at text cell (+0x14, +0x18) in
 * colour +0x1C; +0x10 = next string position, column + 1 */
static void sub_119590(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x10u, a1);
    uint16_t col = rd16(m, obj + 0x14u);
    uint16_t row = rd16(m, obj + 0x18u);
    uint16_t d0 = (uint16_t)(col + col + (uint16_t)(row << 7));
    sub_005666(m, obj + 0x22u, (uint32_t)(int32_t)(int16_t)d0 + 0x0061C000u,
               rd16(m, obj + 0x1Cu));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
}

/* ROM 0x119270: typewriter: one character every 3 frames from +0x10; at the
 * NUL the line is recorded (+0x3A++) and 0x1192A0 waits 0x18 frames */
int Obj_119270_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00119270u);
        return 0;
    }
    sub_1195D0(m, obj);
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint8_t c = rd8(m, a1++);
    wr8(m, obj + 0x22u, c);
    if (c != 0u) {
        sub_119590(m, obj, a1);
        wr16(m, obj + 0x06u, 3u);
        wr32(m, obj + 0x00u, 0x00119270u);
        return 0;
    }
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 1u));
    wr16(m, obj + 0x06u, 0x18u);
    wr32(m, obj + 0x00u, 0x001192A0u);
    return 0;
}
