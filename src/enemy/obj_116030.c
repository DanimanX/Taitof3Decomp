#include "obj_116030.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap hook */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x116030 */
int Obj_116030_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);         /* movea.l (0x8,A0),A1 */
    wr32(m, obj + 0x08u, rd32(m, a1 + 0x08u));  /* move.l (0x8,A1),(0x8,A0) */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x52u, 0xFF88u);              /* move.w #-0x78,(0x52,A0) */
    wr16(m, obj + 0x54u, 0u);

    /* 0x116056: move.w #0x29,-(SP); jsr 0x13F238; jsr 0x39D8 */
    int16_t cmd = sub_13F238(m, 0x0029);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    /* 0x11606A */
    wr16(m, obj + 0x06u, 0x0032u);
    wr32(m, obj + 0x00u, 0x00116078u);
    return 0;
}
