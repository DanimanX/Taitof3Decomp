#include "obj_1150E0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"

#define OBJ_1150E0_FN 0x1150E0u

/* ROM 0x113456: while A5+0x1D58 bit1 is set and x is in [-10, 0x11C], aim at
 * the screen centre (0xA0,0x74) with a random +-5 jitter in +0x42 (restored). */
static void sub_113456(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) return;
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < -0x0A || x > 0x11C) return;

    uint16_t saved = rd16(m, obj + 0x42u);
    uint16_t r = (uint16_t)Rng_NextMem(m);
    wr16(m, obj + 0x42u, (uint16_t)(((r & 0x0Fu)) - 5u));

    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x14u) - 0xA0u);
    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x18u) - 0x74u);
    d0 = Obj_Atan2(m, d0, d1);
    Obj_MoveByAngle(m, obj, (uint8_t)(d0 + 0x58u));
    wr16(m, obj + 0x42u, saved);
}

/* ROM 0x1150E0 */
int Obj_1150E0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) goto L_1152E8;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) goto L_1152E8;

    if ((rd8(m, obj + 0x50u) & 0x08u) != 0u) {
        /* 0x1151AC */
        Obj_Free(m, rd32(m, obj + 0x08u));
        uint32_t c = Obj_Spawn(m, obj, 0x118D4Au, 3u, 2u, 0x1156B2u);
        wr32(m, obj + 0x08u, c);
        Obj_Spawn(m, obj, 0x11555Eu, 3u, 2u, rd32(m, obj + 0x36u));
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr16(m, obj + 0x06u, 0x04B0u);
        wr32(m, obj, 0x11520Cu);
        return 0;
    }

    Obj_BlinkColour(m, obj);

    if ((rd8(m, obj + 0x50u) & 0x01u) == 0u) {
        uint8_t d0b;
        int16_t t = (int16_t)(rd16(m, obj + 0x56u) - 1u);
        wr16(m, obj + 0x56u, (uint16_t)t);
        if (t > 0) {
            uint32_t a1 = rd32(m, obj + 0x3Au);
            uint16_t d0 = (uint16_t)(rd16(m, a1 + 0x14u) + 0x10u - rd16(m, obj + 0x14u));
            uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x18u) + 0x08u - rd16(m, obj + 0x18u));
            d0b = (uint8_t)Obj_Atan2(m, d0, d1);
        } else {
            if ((int16_t)rd16(m, obj + 0x14u) >= 0x1B8) goto L_11530A;
            d0b = 0xF0u;
        }

        /* 0x115146: turn toward the target by at most 3 per tick */
        int8_t diff = (int8_t)(uint8_t)(d0b - rd8(m, obj + 0x20u));
        if (diff < 0) {
            if (diff <= -3) diff = -3;
        } else if (diff >= 3) {
            diff = 3;
        }
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (uint8_t)diff));

        uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
        Obj_MoveByAngle(m, obj, ang);

        uint32_t a1 = 0x115786u;
        uint32_t a2 = rd32(m, obj + 0x0Cu);
        uint16_t idx = (uint16_t)((rd16(m, obj + 0x20u) >> 8) >> 4);
        wr16(m, a2 + 0x52u, rd16(m, a1 + (uint32_t)idx * 4u));
        wr16(m, a2 + 0x54u, rd16(m, a1 + (uint32_t)idx * 4u + 2u));
    }

    /* 0x11519A */
    sub_113456(m, obj);
    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t <= 0) {
            /* 0x1150CC */
            wr16(m, obj + 0x06u, 0x012Cu);
            wr32(m, obj + 0x3Au, rd32(m, F3_A5 + 0x1D28u));
        }
        wr32(m, obj, OBJ_1150E0_FN);
        return 0;
    }

L_1152E8:
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x10u));
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj, 0x115304u);
    return 0;

L_11530A:
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
    Obj_Free(m, rd32(m, obj + 0x08u));
    wr32(m, obj, 0x115326u);
    return 0;
}
