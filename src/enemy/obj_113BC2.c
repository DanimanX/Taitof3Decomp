#include "obj_113BC2.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

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

/* ROM 0x0CBD24: advance the {dur.w, frame.l} animation at a1 by +0x3A/+0x3E */
static uint16_t sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0;

    for (;;) {
        d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* ROM 0x113BC2: animate (0x113C52) while moving along +0x20; free when the
 * animation ends or it leaves the screen; scatter on a bomb */
int Obj_113BC2_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n <= 0)
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x01u));
    sub_1136BC(m, obj);
    uint32_t a1 = 0x00113C52u;
    uint16_t d0 = sub_0CBD24(m, obj, a1);
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) && x >= -0x0A && x <= 0x011C) {
        Obj_ExplodeTick(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    if (out_of(m, 0x000CC112u, rd16(m, obj + 0x14u)) || out_of(m, 0x000CC116u, rd16(m, obj + 0x18u))) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00113BC2u);
    return 0;
}
