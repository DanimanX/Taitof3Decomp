#include "obj_117826.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x1136BC: register as a hazard for player 1 (+0x22 == 0, A5+0x906)
 * or player 2 (A5+0x902), then move along angle +0x20 */
static void sub_1136BC(const f3mem_t *m, uint32_t obj)
{
    uint32_t list = rd16(m, obj + 0x22u) == 0u ? F3_A5 + 0x0906u : F3_A5 + 0x0902u;
    uint32_t a1 = rd32(m, list);
    wr32(m, a1, obj);
    wr32(m, list, a1 + 4u);
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
}

/* cmp2.w (abs).l,Dn; bcs -> out of the signed bounds at addr */
static int out_of(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x117826: flying state; free on terrain contact / off-screen */
int Obj_117826_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x117826 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || (int32_t)rd32(m, obj + 0x24u) <= 0)
        goto free_obj;

    /* 0x117838 */
    if (Terrain_TestPoint(m, (int16_t)(rd16(m, obj + 0x14u) + 0x0008u),
                             (int16_t)(rd16(m, obj + 0x18u) + 0x0008u)))
        goto free_obj;

    /* 0x117850 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);        /* jmp 0x0CBB2C */
            return 0;
        }
    }

    /* 0x11786E */
    sub_1136BC(m, obj);
    Obj_DrawSprite(m, obj);

    /* 0x11787A: cmp2.w bounds at 0x0CC112 / 0x0CC116 */
    if (out_of(m, 0x000CC112u, rd16(m, obj + 0x14u)))
        goto free_obj;
    if (out_of(m, 0x000CC116u, rd16(m, obj + 0x18u)))
        goto free_obj;

    /* 0x11789A -> 0x11781E */
    wr32(m, obj + 0x00u, 0x00117826u);
    return 0;

free_obj:
    /* 0x11789C */
    Obj_Free(m, obj);
    return 0;
}
