#include "obj_D77C4.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D77C4: spinning pod; animates, bobs on +0x5C, advances +0x6 and hands over to 0x0D78C0 */
int Obj_D77C4_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die_fx;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    die_fx:
        Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)rd8(m, obj + 0x65u)));
        uint32_t d0 = Obj_13E4D0(m, obj, 2u);
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0);
        wr32(m, obj + 0x00u, 0x000D79B0u);
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    /* SUB_0D7D48: bob the Y position */
    {
        uint16_t d0 = rd16(m, obj + 0x5Cu);
        if ((d0 & 1u) != 0u) {
            wr16(m, obj + 0x5Cu, (uint16_t)(rd16(m, obj + 0x5Cu) + 0x222u));
            if ((int16_t)rd16(m, obj + 0x5Cu) >= 0x4000)
                d0 ^= 1u;
        } else {
            wr16(m, obj + 0x5Cu, (uint16_t)(rd16(m, obj + 0x5Cu) - 0x222u));
            if ((int16_t)rd16(m, obj + 0x5Cu) <= -0x4000)
                d0 ^= 1u;
        }
        /* bfins D0,(0x5C,A0){0xf:0x1}: bit 0 of the word (field offset 15, width 1) = D0 bit 0 */
        wr16(m, obj + 0x5Cu, (uint16_t)((rd16(m, obj + 0x5Cu) & 0xFFFEu) | (d0 & 1u)));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)(int32_t)(int16_t)d0);
    }

    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    Obj_AnimFrameStep(m, obj, 0x000D79D8u);
    {
        uint16_t idx = rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, 0x000D79D8u + idx) > 0) {
            wr32(m, obj + 0x00u, 0x000D77C4u);
            return 0;
        }
    }

    if ((rd8(m, obj + 0x64u) & 1u) == 0u) {
        wr8(m, obj + 0x64u, (uint8_t)(rd8(m, obj + 0x64u) | 1u));
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 1u));
        wr32(m, obj + 0x10u, 0x0006423Cu);
        wr16(m, obj + 0x20u, (uint16_t)((uint16_t)(rd16(m, obj + 0x20u) - 0x4000u) & 0x8000u));
    }

    {
        uint8_t old = rd8(m, obj + 0x06u);
        int16_t step = (int16_t)rd16(m, obj + 0x60u) >> 3;
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + (uint16_t)step));
        if (rd8(m, obj + 0x06u) != old) {
            uint32_t d1 = ((rd8(m, obj + 0x1Cu) & 1u) == 0u) ? 0x0D1D8000u : 0x121D0000u;
            (void)Obj_Spawn(m, obj, 0x000CD956u, 9u, 2u, d1);
            Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x2A));
        }
    }

    if ((int8_t)rd8(m, obj + 0x06u) >= 3) {
        wr16(m, obj + 0x62u, 0u);
        wr32(m, obj + 0x00u, 0x000D78C0u);
    } else {
        wr32(m, obj + 0x00u, 0x000D77C4u);
    }
    return 0;
}
