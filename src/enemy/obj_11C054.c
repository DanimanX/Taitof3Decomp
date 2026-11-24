#include "obj_11C054.h"
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

/* ROM 0x11BFE6, entered from 0x11C054 */
static int sub_11BFE6(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }

    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));

    if ((Rng_NextMem(m) & 0x1Fu) == 0u) {
        Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, rd32(m, obj + 0x0Cu));
        if ((Rng_NextMem(m) & 0x0Fu) == 0u)
            play_sound(m, 0x0033u);
    }

    wr32(m, obj + 0x00u, 0x0011C054u);
    return 0;
}

/* ROM 0x11C054 */
int Obj_11C054_Tick(const f3mem_t *m, uint32_t obj)
{
    return sub_11BFE6(m, obj);
}
