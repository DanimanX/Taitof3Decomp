#include "obj_D93D4.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* bomb active and on screen: bomb-scatter tail (jmp 0xCBB2C) */
static int bombed(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    return (rd8(m, F3_A5 + 0x1D58u) & 0x02u) && x >= -0x0A && x <= 0x011C;
}

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0D93D4: scripted, scrolling, shrinking/growing (+0x1E) enemy shot */
int Obj_D93D4_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    uint8_t f7 = rd8(m, obj + 0x07u);
    wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
    if ((f7 & 0x02u) && ObjScript_CF1E4(m, obj) < 0)
        return -1;
    if (rd8(m, obj + 0x20u) != 0u && ObjScript_CF4B8(m, obj, 0x000D94B2u) < 0)
        return -1;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)rd16(m, obj + 0x14u), (int16_t)rd16(m, obj + 0x18u)) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd16(m, obj + 0x68u) == 0u && !(rd8(m, F3_A5 - 0x144Bu) & 0x20u))
        Coll_AddHazard(m, obj);
    uint16_t d1 = (uint16_t)(rd8(m, obj + 0x1Eu) >> 4);
    uint16_t d0 = (uint16_t)(0x10u - d1);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
    Obj_DrawSprite(m, obj);
    d0 = (uint16_t)(8u - (d1 >> 1));
    wr16(m, obj + 0x2Eu, d0);
    wr16(m, obj + 0x32u, d0);
    wr16(m, obj + 0x2Cu, (uint16_t)(0u - d0));
    wr16(m, obj + 0x30u, (uint16_t)(0u - d0));
    if (bombed(m, obj)) {
        Obj_ExplodeTick(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    return 0;
}
