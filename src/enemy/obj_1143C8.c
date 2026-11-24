#include "obj_1143C8.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0CD3D6: switch to the follow-up record 0x0CD444[D0] */
static void next_record(const f3mem_t *m, uint32_t obj, uint16_t d0)
{
    uint32_t a1 = rd32(m, 0x000CD444u + (uint32_t)(int32_t)(int16_t)d0 * 4u);
    /* record 0x0CD444[1] is read with its leading sound word skipped */
    a1 += 2u;
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x00u, 0x000CD412u);
}

/* ROM 0x1142CE: falling shot body. It leaves early when the life counter ran
 * out, the global pause bit is set or the +8 terrain probe hits; otherwise it
 * mirrors the grandparent flags, animates through 0x1143E4, sinks while +0x6
 * lasts, registers into the player-2 or hazard list, moves and draws, and
 * finally checks the 0x0CC112 screen bounds. */
static int body(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        next_record(m, obj, 1u);
        return 0;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        next_record(m, obj, 1u);
        return 0;
    }
    if (Terrain_TestPoint(m, (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + 8u),
                             (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + 8u)) != 0) {
        next_record(m, obj, 1u);
        return 0;
    }

    uint32_t gp = rd32(m, rd32(m, obj + 0x08u) + 0x08u); /* *(*(obj+8)+8) */
    if ((rd8(m, gp + 0x50u) & 0x08u) == 0u) {
        if ((rd8(m, gp + 0x50u) & 0x01u) != 0u ||
            (rd8(m, gp + 0x50u) & 0x10u) != 0u) {
            next_record(m, obj, 1u);
            return 0;
        }
    }
    if ((rd8(m, gp + 0x50u) & 0x20u) != 0u) {
        next_record(m, obj, 1u);
        return 0;
    }

    Obj_Animate(m, obj, 0x001143E4u);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            /* 0x11434C: hand over to the bomb orbit */
            wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u)));
            return -1;
        }
    }

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        /* 0x11435A: sink */
        wr16(m, obj + 0x1Eu, (uint16_t)(rd16(m, obj + 0x1Eu) - 0x0808u));
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00006000u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x00006000u);
    } else {
        /* 0x114374 */
        if (rd16(m, obj + 0x0Cu) == 0u) {
            Coll_AddHazard(m, obj);
        } else {
            uint32_t a1 = rd32(m, F3_A5 + 0x0902u);
            wr32(m, a1, obj);
            wr32(m, F3_A5 + 0x0902u, a1 + 4u);
        }
        Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    }

    Obj_DrawSprite(m, obj);

    /* 0x1143A0: cmp2.w compares against the (lo,hi) pair at 0x0CC112 / 0x0CC116
     * and bcs means "inside the range". At 0x0CC112 both bounds are 0xFFE0, so
     * no coordinate ever falls inside and this path never releases the object. */
    {
        uint16_t lo = rd16(m, 0x000CC112u), hi = rd16(m, 0x000CC114u);
        uint16_t x  = rd16(m, obj + 0x14u);
        if (x >= lo && x <= hi) {
            Obj_Free(m, obj);
            return 0;
        }
        uint16_t lo2 = rd16(m, 0x000CC116u), hi2 = rd16(m, 0x000CC118u);
        uint16_t y   = rd16(m, obj + 0x18u);
        if (y >= lo2 && y <= hi2) {
            Obj_Free(m, obj);
            return 0;
        }
    }
    wr32(m, obj + 0x00u, 0x001143C8u);
    return 0;
}

/* ROM 0x1143C8: the pool slot parks on this thunk, which re-runs 0x1142CE */
int Obj_1143C8_Tick(const f3mem_t *m, uint32_t obj)
{
    return body(m, obj);
}
