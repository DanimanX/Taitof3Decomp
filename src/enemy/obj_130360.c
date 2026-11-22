#include "obj_130360.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CBD86: clear flip/zoom/speed/anim state */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x0CBEB0: D0.w = ((A5+0x87E)*8 + 0xC00) *s D0.w, <<4, swap */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 3);
    d1w = (uint16_t)(d1w + 0x0C00u);
    int32_t p = (int32_t)(int16_t)d1w * (int32_t)(int16_t)d0w;
    uint32_t d0 = (uint32_t)p << 4;
    d0 = (d0 << 16) | (d0 >> 16);
    return (uint16_t)d0;
}

/* ROM 0x13F238: sound command remap */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x130360 */
int Obj_130360_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);                             /* 0x130360 */

    wr16(m, obj + 0x28u, 1u);                       /* 0x130366 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));   /* bset.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u));   /* bset.b #3 */

    /* 0x130378: Sound_PlayCmd(0x23) */
    int16_t cmd = sub_13F238(m, 0x0023);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    /* 0x13038C */
    uint16_t d0w = sub_0CBEB0(m, 0x0096);
    wr16(m, obj + 0x22u, d0w);
    wr16(m, obj + 0x06u, 0x00C8u);
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - d0w));

    /* 0x1303A4 */
    wr32(m, obj + 0x00u, 0x001303ACu);
    return 0;
}
