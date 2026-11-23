#include "obj_114984.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

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

/* ROM 0x114984 */
int Obj_114984_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    sub_1136BC(m, obj);
    Obj_Animate(m, obj, 0x001149FCu);   /* lea (0x40,PC) at 0x1149BA */
    Obj_DrawSprite(m, obj);
    if (out_of(m, 0x000CC112u, rd16(m, obj + 0x14u)) || out_of(m, 0x000CC116u, rd16(m, obj + 0x18u))) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00114984u);
    return 0;
}
