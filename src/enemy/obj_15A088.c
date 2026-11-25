#include "obj_15A088.h"
#include "../objects/obj_common.h"

/* ROM 0x15A088: palette/fade step + Pal_QueueRamp */
int Obj_15A088_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x44u) - 1u);
    wr16(m, obj + 0x44u, (uint16_t)t);
    if (t >= 0) {
        if ((t & 0x1F) == 0) {
            int16_t d0w = (int16_t)(t >> 5);
            int16_t d1w = 4;
            uint32_t a0 = (uint32_t)(0x00626200 + (int16_t)rd16(m, F3_A5 - 0x5D56u));
            d1w &= 0x0F;
            if (d1w > 8) d1w = 8;
            d0w &= 0x0F;
            if (d0w > 8) d0w = 8;
            d1w = (int16_t)(((d1w << 8) + (8 - d1w)) << 4);
            d0w = (int16_t)((d0w << 8) + (8 - d0w));
            uint16_t val = (uint16_t)(~(d1w | d0w));
            for (uint32_t i = 0u; i <= 0xE7u; i++)
                wr16(m, a0 + i * 2u, val);
        }
        wr32(m, obj + 0x00u, 0x0015A088u);
        return 0;
    }
    Pal_QueueRamp(m, 0x0015A492u, 0x0015A516u, 0u, 0xB4u, 1u);
    wr32(m, obj + 0x00u, 0x0015A0C8u);
    return 0;
}
