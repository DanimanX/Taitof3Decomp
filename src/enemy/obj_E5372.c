#include "obj_E5372.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

/* ROM 0x0E5372: scrolling hazard/enemy tick with 4-frame +0x21 spin table,
 * HP/bomb handling, and explosion handover. */
int Obj_E5372_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x06u) == 3u) {               /* 0x0E5372 */
        Obj_StartDying(m, obj);                    /* 0x0E5378 */
        return 0;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {      /* 0x0E537E */
        Obj_0D0E04(m, obj, 0x012Cu);               /* 0x0E5404 */
        wr8(m, obj + 0x06u, 0u);                   /* 0x0E540A */
        Obj_StartDying(m, obj);                    /* 0x0E540E */
        return 0;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {         /* 0x0E5386 */
        wr8(m, obj + 0x06u, 0u);                   /* 0x0E540A */
        Obj_StartDying(m, obj);                    /* 0x0E540E */
        return 0;
    }

    Obj_SubScrollVel(m, obj);                      /* 0x0E5390..0x0E539C */

    if (rd8(m, obj + 0x06u) != 2u) {               /* 0x0E53A0 */
        uint8_t d0b = (uint8_t)(rd8(m, obj + 0x21u) + 1u);
        wr8(m, obj + 0x21u, d0b);                  /* 0x0E53B0 */
        uint16_t idx = (uint16_t)((d0b >> 5) & 3u);
        wr8(m, obj + 0x23u, (uint8_t)idx);         /* 0x0E53BA */
        uint32_t a1 = rd32(m, 0x000E5472u + (uint32_t)idx * 4u); /* 0x0E53BE */
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); /* 0x0CFA86..0x0CFA90 */
        wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
        wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
    }

    Obj_0D0DC2(m, obj);                            /* 0x0E53CC */
    Obj_BlinkColour(m, obj);                       /* 0x0E53D2 */

    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {         /* 0x0E53D8 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu)); /* 0x0E53F0 */
            Obj_RelayTimer(m, 0x012Cu);            /* 0x0E541A */
            wr8(m, obj + 0x06u, 0u);               /* 0x0E5420 */
            wr32(m, obj + 0x10u, 0x0005DA8Cu);     /* 0x0E5424 */
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 5u)); /* 0x0E542C */
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 3u)); /* 0x0E5430 */
            Obj_ExplodeTick(m, obj);               /* 0x0E5434 */
            return 0;
        }
    }

    return 0;
}
