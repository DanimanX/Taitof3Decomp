#include "obj_CAEBA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap while A5+0x3306 is set */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x0039D8 call with the pre-hook applied */
static void play_cmd(const f3mem_t *m, int16_t cmd)
{
    cmd = sub_13F238(m, cmd);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0xCAEBA */
int Obj_CAEBA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0CAEBA */
    play_cmd(m, 0x0018);

    /* 0x0CAECE */
    wr32(m, F3_A5 + 0x1D54u, obj);
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x60u, (uint16_t)(rd16(m, a1 + 0x60u) - 1u));
    wr32(m, F3_A5 + 0x1D50u, a1);

    /* 0x0CAEDE: Obj_Spawn(fn 0x0CB406, group 0, ngroups 1, arg 0) */
    (void)Obj_Spawn(m, obj, 0x000CB406u, 0u, 1u, 0u);

    /* 0x0CAEFA */
    Obj_ResetFields(m, obj);
    wr16(m, F3_A5 + 0x1D5Au, 0u);
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x10u, 0x000625F4u);

    /* 0x0CAF12 */
    wr32(m, obj, 0x000CAF1Au);
    return 0;
}
