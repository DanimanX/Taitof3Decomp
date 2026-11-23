#include "obj_1242EC.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap hook */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x11C35E: set (or restore) the sound id at +0x1D. Can the routine go
 * 0x11C384 -> jsr 0x000039D8 (Sound_PlayCmd 0x1C)? The capture for
 * 0x1242EC (60 pairs) never takes it: A5+0x3306 is 0 and A0+0x35 is 0 in
 * every pair, while all 60 do take the 0x11C398 path (the +0x35 branch
 * writes 0x1C, the 0x11C392 branch copies +0x4F, which is not 0x1C). */
static int sub_11C35E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u && rd8(m, obj + 0x1Du) != 0x10u) {
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, 0x10u);
        int16_t cmd = sub_13F238(m, 0x001C);
        if (m->dpram) {
            Sound_PlayCmd(m, cmd);
        } else {
            uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
            f3mem_t tmp_m = *m;
            tmp_m.dpram = tmp_dpram;
            Sound_PlayCmd(&tmp_m, cmd);
        }
        return 0;
    }
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    return 0;
}

/* ROM 0x1242EC: end-of-game check, switches to 0x12431E when it survives. */
int Obj_1242EC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1242EC */
    if (sub_11C35E(m, obj) < 0)
        return -1;

    /* 0x1242F2: still in the final 0x3840 frames? */
    if ((int16_t)rd16(m, F3_A5 + 0x3318u) <= 0x3840) {
        /* 0x1242FC */
        wr8(m, F3_A5 + 0x30F4u, (int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x3C ? 0xFFu : 0x00u); /* sle */
        if ((int32_t)rd32(m, F3_A5 + 0x30DAu) > 0) {
            /* 0x1242E4 */
            wr32(m, obj + 0x00u, 0x001242ECu);
            return 0;
        }
    }

    /* 0x12430C */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x10u)); /* bset.b #4 */
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);                                      /* st */
    wr32(m, obj + 0x00u, 0x0012431Eu);
    return 0;
}
