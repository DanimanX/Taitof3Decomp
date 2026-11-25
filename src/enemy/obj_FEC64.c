#include "obj_FEC64.h"
#include "../objects/obj_common.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* ROM 0x0FEC64 */
int Obj_FEC64_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto die;

    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x02u) != 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }

    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x000F824Cu) < 0)
            return -1;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    if ((int16_t)rd16(m, obj + 0x14u) < -0x46)
        goto die;

    uint32_t ring = rd32(m, obj + 0x0Cu);
    uint16_t idx = rd16(m, ring);
    uint32_t dst = rd32(m, ring + 0x04u) + (uint32_t)(int32_t)(int16_t)idx;
    wr16(m, dst, rd16(m, obj + 0x58u));
    wr16(m, ring, (uint16_t)((idx + 2u) & 0x01FFu));

    uint8_t c23 = rd8(m, obj + 0x23u);
    if (c23 != 0u) {
        wr8(m, obj + 0x23u, (uint8_t)(c23 - 1u));
        wr8(m, obj + 0x22u, 0u);
    } else if (rd8(m, obj + 0x22u) != 0u) {
        wr8(m, obj + 0x22u, 0u);
        wr8(m, obj + 0x23u, 7u);
        int16_t v = (int16_t)(rd16(m, obj + 0x08u) - 1u);
        wr16(m, obj + 0x08u, (v < 0) ? 0u : (uint16_t)v);
    } else if (rd16(m, obj + 0x08u) != 0x000Eu) {
        wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) + 0x00004E20u);
    }

    uint32_t cur = rd32(m, obj + 0x54u);
    do {
        if (rd8(m, cur + 0x06u) == 0u) {
            wr32(m, cur + 0x18u, rd32(m, cur + 0x18u) - rd32(m, F3_A5 + 0x4228u));
            Obj_DrawSprite(m, cur);
        }
        cur = rd32(m, cur + 0x54u);
    } while ((int32_t)cur >= 0);

    return 0;

die:
    {
        uint32_t ring = rd32(m, obj + 0x0Cu);
        uint8_t bit = rd8(m, ring + 0x02u) & 7u;
        wr8(m, F3_A5 + 0x1D5Eu, (uint8_t)(rd8(m, F3_A5 + 0x1D5Eu) & (uint8_t)~(1u << bit)));
        wr8(m, obj + 0x06u, 1u);
        Obj_StartDying(m, obj);
        return 0;
    }
}
