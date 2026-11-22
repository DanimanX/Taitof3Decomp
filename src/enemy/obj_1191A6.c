#include "obj_1191A6.h"

/* Start a text line: string, column, next row (+2), colour; record it in
 * the line table at +0x44 + line*8 {col.w, row.w, colour:8 | string:24} */
static void line_start(const f3mem_t *m, uint32_t obj, uint32_t str, uint16_t col,
                       uint16_t colour, uint32_t self)
{
    wr32(m, obj + 0x10u, str);
    wr16(m, obj + 0x14u, col);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
    wr16(m, obj + 0x1Cu, colour);
    uint32_t a1 = obj + 0x44u + (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x3Au) * 8);
    wr16(m, a1, col);
    wr16(m, a1 + 2u, rd16(m, obj + 0x18u));
    wr32(m, a1 + 4u, str);
    wr8(m, a1 + 4u, rd8(m, obj + 0x1Du));
    wr16(m, obj + 0x06u, 3u);
    wr32(m, obj + 0x00u, self);
}

/* ROM 0x1191A6: wait +6 frames, then start the per-stage line (0x13F6A0 + stage*25, column 8, colour 8) */
int Obj_1191A6_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001191A6u);
        return 0;
    }
    uint16_t off = (uint16_t)((int32_t)(int16_t)rd16(m, F3_A5 + 0x32DCu) * 0x19);
    line_start(m, obj, 0x0013F6A0u + (uint32_t)(int32_t)(int16_t)off, 8u, 8u, 0x001191FAu);
    return 0;
}
