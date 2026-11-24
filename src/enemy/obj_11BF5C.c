#include "obj_11BF5C.h"
#include "../objects/obj_common.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 -> 0x0039D8: the caller pushes the sound command and the hook
 * rewrites that stack word before Sound_PlayCmd consumes it. */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    Sound_PlayCmd(m, cmd);
}

/* ROM 0x11BF5C */
int Obj_11BF5C_Tick(const f3mem_t *m, uint32_t obj)
{
    play_sound(m, 0x0012u);
    Obj_ResetFields(m, obj);

    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x42u, 0x0006u);

    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0));
    d0 = (d0 << 16) | (d0 >> 16);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x10u));

    wr32(m, obj + 0x00u, 0x0011BFACu);
    return 0;
}
