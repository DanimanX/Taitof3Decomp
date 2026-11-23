#include "obj_115F96.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* cmpi.w (|a1 + 2|).l,D1n ; bcs -> the word is outside the [lo,hi] pair */
static int out_of(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x0CBE12 (ROM: lea 0x27E40,A3; D1 = D2 = (0x42,A0); MULS.W
 * (0x80,A3,D0.W*2),D1; ADD.L D1,(0x14,A0); MULS.W (A3,D0.W*2),D2;
 * ADD.L D2,(0x18,A0)) -- the local Obj_MoveByAngle uses the wrong
 * address register (obj+0x42 instead of (0x42,A0)) and adds 32-bit
 * products, so it is re-expressed here register-faithfully. */
static void sub_0CBE12(const f3mem_t *m, uint32_t obj, uint16_t angle)
{
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)(int16_t)angle * 2u);
    int16_t sin_v = (int16_t)rd16(m, 0x027E40u + (uint32_t)(int16_t)angle * 2u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)(int32_t)((int32_t)spd * (int32_t)cos_v));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)(int32_t)((int32_t)spd * (int32_t)sin_v));
}

/* ROM 0x1136BC: put the object on the hazard list of its player (+0x22 == 0
 * -> A5+0x906, else A5+0x902), then move along angle (0x20,A0) */
static void sub_1136BC(const f3mem_t *m, uint32_t obj)
{
    uint32_t list = rd16(m, obj + 0x22u) == 0u ? F3_A5 + 0x0906u : F3_A5 + 0x0902u;
    uint32_t a1 = rd32(m, list);
    wr32(m, a1, obj);
    wr32(m, list, a1 + 4u);
    sub_0CBE12(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
}

/* ROM 0x115F96: spinning bullet launched by 0x115F36.
 *   115F96  btst.b #5,(-0x144b,A5) ; bne free
 *   115FA0  tst.l (0x24,A0) ; ble free
 *   115FA8  Terrain_TestPoint(x+8, y+8) ; D0b != 0 -> free
 *   115FC0  while the A5+0x1D58 bit1 is set and x in [-0xA,0x11C] -> 0x0CBB2C
 *   115FDE  sub_1136BC ; Obj_Animate(0x115FDE,PC+0x42 -> 0x11602A) ; draw
 *   115FF6  cmp2.w (0xcc112).l / (0xcc116).l on x / y ; bcs -> free
 *   116016  move.l #0x115f96,(A0)
 *   11601A  Obj_Free */
int Obj_115F96_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x115F96 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x115FA8 */
    if (Terrain_TestPoint(m, (int16_t)(rd16(m, obj + 0x14u) + 8u),
                          (int16_t)(rd16(m, obj + 0x18u) + 8u)) != 0) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x115FC0 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);               /* ROM 0x0CBB2C */
            return 0;
        }
    }

    /* 0x115FDE */
    sub_1136BC(m, obj);
    Obj_Animate(m, obj, 0x00116028u);              /* lea (0x42,PC) at 0x115FE4 */

    /* 0x115FF0 */
    Obj_DrawSprite(m, obj);
    if (out_of(m, 0x000CC112u, rd16(m, obj + 0x14u)) || out_of(m, 0x000CC116u, rd16(m, obj + 0x18u))) {
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x116016 */
    wr32(m, obj + 0x00u, 0x00115F96u);
    return 0;
}
