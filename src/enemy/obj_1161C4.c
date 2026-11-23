#include "obj_1161C4.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../enemy/obj_E8B72.h"
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

/* ROM 0x0CD3D6: switch to the follow-up record 0x0CD444[D0] (optional
 * sound word first), restart animation, next state 0x0CD412 */
static void next_record(const f3mem_t *m, uint32_t obj, uint16_t d0)
{
    uint32_t a1 = rd32(m, 0x000CD444u + (uint32_t)(int32_t)(int16_t)d0 * 4u);
    uint16_t snd = rd16(m, a1); a1 += 2u;
    if (snd != 0u)
        Sound_PlayHooked(m, snd);
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x00u, 0x000CD412u);
}

/* ROM 0x1161C4: shot/missile body (animation table at 0x116270) */
int Obj_1161C4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1161C4 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || (int32_t)rd32(m, obj + 0x24u) <= 0)
        goto next;                          /* 0x116238 */

    /* 0x1161D6 */
    sub_1136BC(m, obj);
    Obj_Animate(m, obj, 0x00116270u);

    /* 0x1161E6 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);        /* jmp 0x0CBB2C */
            return 0;
        }
    }

    /* 0x116204 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x04u) != 0u)
        goto next;                          /* 0x116238 */

    /* 0x11620E */
    Obj_DrawSprite(m, obj);
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x > -0x50 && x < 0x0140) {
            wr32(m, obj + 0x00u, 0x001161C4u);   /* 0x116228 -> 0x1161BC */
            return 0;
        }
    }

    /* 0x11622A */
    Obj_Free(m, obj);
    return 0;

next:
    /* 0x116238 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0020u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0008u));
    next_record(m, obj, 1u);
    return 0;
}
