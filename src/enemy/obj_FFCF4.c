#include "obj_FFCF4.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0FFCF4: multi-part boss/enemy body tick */
int Obj_FFCF4_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x3Eu) <= 0x73 &&
        (rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x00000FA0u);
            goto spawn_frags;
        }
    }
    if (rd8(m, obj + 0x06u) == 3u)
        goto spawn_frags;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        if (rd8(m, obj + 0x35u) == 1u || rd8(m, obj + 0x35u) == 2u)
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00000FA0u);
        goto spawn_frags;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto spawn_frags;
    if ((int16_t)rd16(m, obj + 0x3Au) < -0x60) {
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    if ((rd8(m, obj + 0x07u) & 0x02u) != 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & ~0x02u));
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }
    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x000FFFE2u) < 0)
            return -1;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    switch (rd8(m, obj + 0x06u)) {
    case 1u:
        if (rd8(m, obj + 0x35u) != 0u) {
            wr8(m, obj + 0x23u, 0x0Au);
            wr8(m, obj + 0x06u, 2u);
        }
        break;
    case 2u: {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u)
            wr8(m, obj + 0x06u, 1u);
        uint32_t d0 = 0u, d1 = 0u;
        if (Obj_0CF898(m, obj, &d0, &d1) < 0)
            return -1;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
        break;
    }
    default:
        return -1;
    }

    int16_t d0w = (int16_t)(rd16(m, obj + 0x18u) + 7);
    if (d0w <= 0xC3) {
        d0w = (int16_t)(-(d0w - 0xC3) + 7);
        if ((uint16_t)d0w > 0x41u) {
            Obj_BlinkColour(m, obj);
            d0w = 0x41;
        }
        wr16(m, obj + 0x32u, (uint16_t)d0w);
        Obj_0D0DC2(m, obj);
    }
    if (rd8(m, obj + 0x22u) != 0u && Obj_Next4Way(m) >= 0x96u) {
        uint32_t c = Obj_Spawn(m, obj, 0x001008A6u, 5u, 2u, 0u);
        if ((int32_t)c >= 0) {
            wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 0x28u));
            wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 0x28u));
        }
    }
    wr32(m, obj + 0x00u, 0x000FFCF4u);
    return 0;

spawn_frags:
    ;
    static const struct { uint16_t dx, dy, ang, kind; } frags[5] = {
        { 0x0000u, 0x0000u, 0x0087u, 0u },
        { 0x0030u, 0x0000u, 0x002Du, 1u },
        { 0x0000u, 0x0025u, 0x00E1u, 2u },
        { 0x0012u, 0x0026u, 0x010Eu, 3u },
        { 0x002Cu, 0x0026u, 0x013Bu, 4u },
    };
    for (int i = 0; i < 5; i++) {
        uint32_t c = Obj_Spawn(m, obj, 0x001003E2u, 5u, 2u, 0u);
        if ((int32_t)c < 0)
            break;
        wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + frags[i].dx));
        wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + frags[i].dy));
        wr16(m, c + 0x60u, 1u);
        wr16(m, c + 0x58u, frags[i].ang);
        wr16(m, c + 0x10u, frags[i].kind);
    }
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0025));
    wr8(m, obj + 0x06u, 0u);
    Obj_StartDying(m, obj);
    return 0;
}
