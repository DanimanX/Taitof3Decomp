#include "obj_10010C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"
#include "obj_E8B72.h"

/* ROM 0x0D0DC2: append A0 to the hazard list at A5+0x8FE, then (fall through
 * to 0x0D0DCC) the tail of Coll_AddHazard. */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x8FEu, a1);
    Coll_AddHazard(m, obj);
}

/* ROM 0x10010C */
int Obj_10010C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x10010C */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        /* 0x1001D0 */
        if ((int16_t)rd16(m, obj + 0x14u) < (int16_t)-0x50) {
            Obj_StartDying(m, obj);
            return 0;
        }
        return 0;
    }

    /* 0x100118 */
    if ((rd8(m, 0xF3EB5u) & 0x20u) != 0u) {
        /* 0x1001C2 */
        wr8(m, rd32(m, obj + 0x08u) + 0x06u, 3u);
        /* falls through to 0x1001DC */
    } else if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        /* 0x10012A */
        uint32_t a4 = obj;
        uint32_t a0 = rd32(m, obj + 0x08u);
        Obj_DrawSprite(m, a0);

        /* 0x100136 */
        a1 = rd32(m, obj + 0x08u);
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x1Cu));
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x5Eu));
        wr8(m, obj + 0x4Fu, (uint8_t)-0x12);

        /* 0x10015A */
        Obj_StepScript(m, obj);
        Obj_BlinkColour(m, obj);
        Obj_DrawSprite(m, obj);

        /* 0x10016C */
        a1 = rd32(m, obj + 0x08u);
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
        wr8(m, obj + 0x4Fu, (uint8_t)-0x34);
        Obj_BlinkColour(m, obj);

        /* 0x100188 */
        a4 = rd32(m, obj + 0x10u);
        wr32(m, obj + 0x10u, 0x00065A68u);
        Obj_DrawSprite(m, obj);
        wr32(m, obj + 0x10u, a4);

        /* 0x10019E */
        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x18u) + 0x48u);
        if ((int16_t)d0 <= (int16_t)0xA5)
            sub_0D0DC2(m, obj);
        return 0;
    } else {
        /* 0x1001B6 */
        Score_OnHit(m, obj, 0x190u);
        /* falls through to 0x1001C2 */
        wr8(m, rd32(m, obj + 0x08u) + 0x06u, 3u);
    }

    /* 0x1001DC */
    uint32_t spawned = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);
    /* 0x1001F8 */
    if ((int32_t)spawned < 0) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x100200 */
    a1 = spawned;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x24u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x50u));
    wr16(m, a1 + 0x58u, 0u);
    /* 0x10021E: clr.w (0x60,A1) leaves Z set, so the following 67 FF FF FC
     * (beq 0x100222) never branches: falls through to 0x0CEE74. */
    wr16(m, a1 + 0x60u, 0u);
    Obj_StartDying(m, obj);
    return 0;
}
