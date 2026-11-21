#include "obj_119714.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x119714 */
int Obj_119714_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3au);
    int to_proc = 0;

    for (;;) {
        uint16_t d0 = rd16(m, a1);
        a1 += 2u;
        if ((int16_t)d0 < 0) {
            break;
        }
        if ((int16_t)d0 > (int16_t)rd16(m, obj + 0x3eu)) {
            to_proc = 1;
            break;
        }
        d0 = rd16(m, a1);
        a1 += 2u;
        uint32_t d1 = 0x01400000u | 0x80000000u;
        d1 = (d1 & ~0x70000000u) | ((d0 & 7u) << 28);
        uint32_t a2 = obj + 0x44u;
        uint16_t dbf = 5u;
        for (;;) {
            if ((rd8(m, a2) & 0x80u) == 0u) {
                wr32(m, a2, d1);
                break;
            }
            a2 += 4u;
            if (dbf-- == 0u) {
                break;
            }
        }
        wr32(m, obj + 0x3au, a1);
    }

    if (!to_proc) {
        uint32_t scan = obj + 0x44u;
        int active = 0;
        uint16_t dbf = 5u;
        for (;;) {
            if (rd32(m, scan) & 0x80000000u) {
                active = 1;
                break;
            }
            scan += 4u;
            if (dbf-- == 0u) {
                break;
            }
        }
        if (!active) {
            Obj_Free(m, obj);
            return 0;
        }
    }

    {
        uint32_t tab = 0x0011980eu;
        uint16_t d0w = 3u;
        for (;;) {
            uint32_t a2 = obj + 0x44u;
            uint16_t d1w = 5u;
            for (;;) {
                uint32_t d2 = rd32(m, a2);
                if (d2 & 0x80000000u) {
                    uint32_t d3 = (d2 >> 28u) & 7u;
                    if ((uint16_t)d3 == d0w) {
                        uint32_t a3 = rd32(m, tab + d3 * 4u);
                        wr16(m, obj + 0x42u, rd16(m, a3));
                        a3 += 2u;
                        wr16(m, obj + 0x18u, (uint16_t)(int8_t)rd8(m, a3));
                        a3 += 1u;
                        wr8(m, obj + 0x1du, rd8(m, a3));
                        a3 += 1u;
                        wr32(m, obj + 0x10u, rd32(m, a3));
                        a3 += 4u;
                        uint32_t pos = (uint32_t)((int32_t)(d2 << 4u) >> 4u);
                        wr32(m, obj + 0x14u, pos);
                        Obj_DrawSprite(m, obj);
                        uint32_t acc =
                            ((uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x42u) << 2u) +
                            rd32(m, obj + 0x14u);
                        if ((int32_t)acc <= -0x00700000) {
                            wr8(m, a2, (uint8_t)(rd8(m, a2) & 0x7fu));
                        } else {
                            d2 = (d2 & 0xf0000000u) | (acc & 0x0fffffffu);
                            wr32(m, a2, d2);
                        }
                    }
                }
                a2 += 4u;
                if (d1w-- == 0u) {
                    break;
                }
            }
            if (d0w-- == 0u) {
                break;
            }
        }
        wr16(m, obj + 0x3eu, (uint16_t)(rd16(m, obj + 0x3eu) + 1u));
    }

    wr32(m, obj, 0x00119714u);
    return 0;
}
