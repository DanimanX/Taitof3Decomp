#include "obj_17E89E.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 -> 0x0039D8, with the pre-command hook applied to the stack
 * word the caller pushed. */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    Sound_PlayCmd(m, cmd);
}

/* ROM 0x17E89E */
int Obj_17E89E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0017E89Eu);
        return 0;
    }

    play_sound(m, 0x002Au);

    uint32_t a1 = 0x0017EBCEu;
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x4Au, 0x0000u);

    for (uint16_t d7 = 7u; ; d7--) {
        Obj_Spawn(m, obj, 0x0017EA0Au, 9u, 2u, d7);
        if (d7 == 0u)
            break;
    }

    wr32(m, obj + 0x00u, 0x0017E99Au);
    return 0;
}
