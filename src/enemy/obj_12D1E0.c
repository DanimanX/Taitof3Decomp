#include "obj_12D1E0.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"

/* ROM 0x12D1D8: hold in this state */
static void sub_12D1D8(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D1E0u);
}

/* ROM 0x13F238: sound command remap */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x12D1E0: step the boss timer up, scroll down at double speed */
int Obj_12D1E0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12D1E0 */
    if ((int16_t)rd16(m, F3_A5 + 0x32A6u) < 4)
        wr16(m, F3_A5 + 0x32A6u, (uint16_t)(rd16(m, F3_A5 + 0x32A6u) + 1u));

    /* 0x12D1EE */
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) - 0x0210u));
    if ((int16_t)rd16(m, F3_A5 + 0x32A2u) != (int16_t)0x8000) {
        /* 0x12D1FA: bne 0x12D1D8 */
        sub_12D1D8(m, obj);
        return 0;
    }

    /* 0x12D1FC */
    wr8(m, F3_A5 + 0x30F3u, 0x00u);   /* sf */
    int16_t cmd = sub_13F238(m, 0x0013);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    /* 0x12D214 */
    wr8(m, F3_A5 + 0x32AEu, 0xFFu);   /* st */
    wr16(m, F3_A5 + 0x32A6u, 6u);
    wr16(m, F3_A5 + 0x32A8u, 2u);

    /* 0x12D224 */
    wr32(m, obj + 0x00u, 0x0012D22Cu);
    return 0;
}
