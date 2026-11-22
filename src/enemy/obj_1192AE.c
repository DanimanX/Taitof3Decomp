#include "obj_1192AE.h"
#include "../objects/obj_pool.h"

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

/* ROM 0x1192AE: draw texts each frame; on the exit flag blank three rows */
int Obj_1192AE_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_1195D0(m, obj);
    if (!(rd8(m, F3_A5 - 0x144Bu) & 0x08u)) {
        wr32(m, obj + 0x00u, 0x001192AEu);
        return 0;
    }
    sub_005666(m, 0x00119316u, 0x0061C816u, 7u);
    sub_005666(m, 0x00119316u, 0x0061C910u, 7u);
    sub_005666(m, 0x00119316u, 0x0061CA16u, 7u);
    Obj_Free(m, obj);
    return 0;
}
