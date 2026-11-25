#include "obj_FE5BA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0FE5BA: script enemy. On screen it flashes and hands over to the
 * explosion tick (0x0CBB2C); alive it re-bases its position from its own
 * velocity minus the scroll delta, flags itself offscreen, dies on a terrain
 * hit or once flagged, and otherwise steps its script. Anywhere else it is
 * killed outright. */
int Obj_FE5BA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0FE5BA: btst.b #0x1,(0x1d58,A5) / cmpi.w #-0xa,(0x14,A0) /
     *          cmpi.w #0x11c,(0x14,A0) */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            /* 0x0FE5D2: move.b (0x4f,A0),(0x1d,A0) ; jmp 0x0CBB2C */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    /* 0x0FE5DE: tst.l (0x24,A0) ; ble.w 0x0FE654
     *           btst.b #0x5,(-0x144b,A5) ; bne.w 0x0FE654 */
    if ((int32_t)rd32(m, obj + 0x24u) > 0 && (rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        /* 0x0FE5F0: 16.16 position += (own velocity - scroll velocity) */
        uint32_t d0 = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
        d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d0);

        /* 0x0FE608 */
        Obj_FlagOffscreen(m, obj);

        /* 0x0FE60E: btst.b #0x3,(0x7,A0) ; bne.l 0x0CEE74 */
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }

        /* 0x0FE61A: move.l (0x3a,A0),(0x14,A0) / (0x3e,A0),(0x18,A0) */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

        /* 0x0FE626: terrain test at pos + (8,8); tst.b D0b ; bne 0x0CEE74 */
        {
            uint16_t d0 = (uint16_t)(rd16(m, obj + 0x14u) + 0x0008u);
            uint16_t d1 = (uint16_t)(rd16(m, obj + 0x18u) + 0x0008u);
            int r = Terrain_TestPoint(m, (int16_t)d0, (int16_t)d1);
            if (r < 0)
                return -1;
            if ((uint8_t)r != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
        }

        /* 0x0FE640: jsr 0x0D0690 / 0x0D0DCC / 0x001282 */
        Obj_StepScript(m, obj);
        Coll_AddHazard(m, obj);
        Obj_DrawSprite(m, obj);
        return 0;
    }

    /* 0x0FE654: pea 0 / move.w #2 / #9 / #0xDBE98 ; jsr 0x01057C
     * then lea (0xc,SP),SP ; bra.l 0x0CEE74 */
    (void)Obj_Spawn(m, obj, 0x000DBE98u, 2u, 9u, 0u);
    Obj_StartDying(m, obj);
    return 0;
}
