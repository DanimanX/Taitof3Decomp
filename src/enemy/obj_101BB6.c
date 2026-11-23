#include "obj_101BB6.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"
#include "obj_E8B72.h"

/* ROM 0x0CEF0E: off-screen handling; in range it draws the explosion
 * (0x0CBB2C), otherwise it parks the object on 0x0CEF44. */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);
}

/* ROM 0x0D0DC2: append obj to A5+0x8FE, then Coll_AddHazard (0x0D0DCC) */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    Coll_AddEnemyHazard(m, obj);
}

/* ROM 0x101BB6 */
int Obj_101BB6_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x101BB6: on screen -> keep alive, just relay the timer */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            /* 0x101BCE */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x101CA2: moveq 0x64,D0 ; jsr 0x0D0E30 ; bra 0x0CEF0E */
            Score_AddCurrent(m, 0x0064u);
            sub_0CEF0E(m, obj);
            return 0;
        }
    }

    /* 0x101BDA */
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
            goto dying;      /* 0x101BE8 -> 0x101C3A */

        /* 0x101BEC */
        uint32_t d0 = rd32(m, obj + 0x50u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
        d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d0);

        /* 0x101C00 */
        Obj_FlagOffscreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);      /* 0x101C0C */
            return 0;
        }

        /* 0x101C12 */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        sub_0D0DC2(m, obj);
        Obj_BlinkColour(m, obj);
        Obj_DrawSprite(m, obj);
        return 0;
    }

    /* 0x101C32: moveq 0x64,D0 ; jsr 0x0D0E04 ; falls through to 0x101C3A */
    Score_OnHit(m, obj, 0x0064u);

dying:
    /* 0x101C3A */
    Sound_PlayHooked(m, 0x001Bu);

    /* 0x101C4E: pea 0 / move.w #2,-(SP) / move.w #3,-(SP) / move.l #0xDC49A,-(SP) */
    uint32_t spawned = Obj_Spawn(m, obj, 0x000DC49Au, 3u, 2u, 0u);
    /* 0x101C6A */
    if ((int32_t)spawned < 0) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x101C72 */
    uint32_t a1 = spawned;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + rd16(m, obj + 0x0Cu)));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + rd16(m, obj + 0x0Eu)));
    wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
    wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
    Obj_StartDying(m, obj);              /* 0x101C9C */
    return 0;
}
