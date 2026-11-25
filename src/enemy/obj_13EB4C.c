#include "obj_13EB4C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x13EB4C: clipped multi-tile laser/beam effect tick */
int Obj_13EB4C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x08u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_AnimFrameStep(m, obj, 0x0013ED1Cu);
    if ((int16_t)rd16(m, 0x0013ED1Cu + rd16(m, obj + 0x3Au)) <= 0) {
        wr32(m, obj + 0x10u, 0x0013EDB0u);
        uint8_t flags = rd8(m, obj + 0x35u);
        wr8(m, obj + 0x35u, (uint8_t)(flags | 0x01u));
        if ((flags & 0x01u) == 0u) {
            uint32_t d0 = ((uint32_t)rd8(m, obj + 0x1Du) << 6) + 0x4004u;
            uint32_t d1 = (rd8(m, obj + 0x21u) >> 4) & 7u;
            d0 = (d0 & 0xFFFF0000u) | (uint16_t)(d0 + rd16(m, 0x0013ED0Cu + d1 * 2u));
            Pal_QueueRamp(m, 0x0013EDF0u, 0x00018F84u, d0, 0x28u, 6u);
        }
        if (((rd16(m, obj + 0x22u) ^ rd16(m, F3_A5 - 0x5D54u)) & 1u) != 0u) {
            wr32(m, obj + 0x00u, 0x0013EB4Cu);
            return 0;
        }
    }
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x32FAu));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x32FCu));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u)));

    uint32_t idx = (rd8(m, obj + 0x21u) >> 4) & 7u;
    uint32_t tbl = rd32(m, obj + 0x10u) + idx * 8u;
    int16_t dx = (int16_t)rd16(m, tbl);
    int16_t dy = (int16_t)rd16(m, tbl + 2u);
    uint32_t spr = rd32(m, tbl + 4u);
    wr32(m, obj + 0x10u, spr);
    uint8_t flip = rd8(m, obj + 0x1Cu);
    if ((flip & 0x02u) != 0u)
        dy = (int16_t)(-dy);
    if ((flip & 0x01u) != 0u)
        dx = (int16_t)(-dx);
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) + dx);
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) + dy);
    wr16(m, obj + 0x14u, (uint16_t)x);
    wr16(m, obj + 0x18u, (uint16_t)y);

    int16_t rows = 0;
    if ((flip & 0x02u) == 0u) {
        if (y <= -0x10)
            goto done;
        int16_t rem = (int16_t)(0x00E8 - y);
        if (rem <= 0)
            goto done;
        rows = (int16_t)((((uint16_t)rem) >> 4) + 1);
        int16_t max_rows = (int16_t)rd16(m, spr);
        if (rows > max_rows)
            rows = max_rows;
    } else {
        if (y >= 0x00E8)
            goto done;
        int16_t rem = (int16_t)(y + 0x10);
        if (rem <= 0)
            goto done;
        rows = (int16_t)((((uint16_t)rem) >> 4) + 1);
        int16_t max_rows = (int16_t)rd16(m, spr);
        if (rows > max_rows)
            rows = max_rows;
        y = (int16_t)(y - (rows << 4));
        wr16(m, obj + 0x18u, (uint16_t)y);
    }

    int16_t cols = 0;
    if ((flip & 0x01u) == 0u) {
        if (x <= -0x10)
            goto done;
        int16_t rem = (int16_t)(0x0140 - x);
        if (rem <= 0)
            goto done;
        cols = (int16_t)((((uint16_t)rem) >> 4) + 1);
        int16_t max_cols = (int16_t)rd16(m, spr + 2u);
        if (cols > max_cols)
            cols = max_cols;
    } else {
        if (x >= 0x0140)
            goto done;
        int16_t rem = (int16_t)(x + 0x10);
        if (rem <= 0)
            goto done;
        cols = (int16_t)((((uint16_t)rem) >> 4) + 1);
        int16_t max_cols = (int16_t)rd16(m, spr + 2u);
        if (cols > max_cols)
            cols = max_cols;
        x = (int16_t)(x - (cols << 4));
        wr16(m, obj + 0x14u, (uint16_t)x);
    }
    Obj_DrawSpriteSub(m, obj, (uint16_t)cols, (uint16_t)rows, 0u);
done:
    wr32(m, obj + 0x00u, 0x0013EB4Cu);
    return 0;
}
