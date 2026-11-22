#include "obj_D8DC4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* cmp2.w (abs).l,Dn; bcs -> outside the signed bounds at addr */
static int out_of(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x0CBDA4: register in the shot-target (A5+0x8FE) and hazard (A5+0x906) lists */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x906u, a1 + 4u);
}

/* ROM 0x0CBDDE: the player credited with the kill (+0x36, via +8 if +0x35 == 1) */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x0D8DC4: scroll with the stage, hittable; on death score 100 and
 * spawn the +0xC list {n.b, args.l...} of 0xD8F4C debris */
int Obj_D8DC4_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t pl = sub_0CBDDE(m, obj);
        wr32(m, pl + 0x48u, rd32(m, pl + 0x48u) + 0x64u);
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        int8_t n = (int8_t)rd8(m, a1++);
        while (--n >= 0) {
            Obj_Spawn(m, obj, 0x000D8F4Cu, 5u, 2u, rd32(m, a1));
            a1 += 4u;
        }
        wr32(m, obj + 0x00u, 0x000D8E42u);
        return 0;
    }
    Obj_SubScrollVel(m, obj);
    if (rd8(m, obj + 0x34u) == 0u)
        sub_0CBDA4(m, obj);
    Obj_DrawSprite(m, obj);
    if ((int16_t)rd16(m, obj + 0x14u) <= -0x80 || out_of(m, 0x000CC116u, rd16(m, obj + 0x18u))) {
        wr32(m, obj + 0x00u, 0x000D8E42u);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x000D8DC4u);
    return 0;
}
