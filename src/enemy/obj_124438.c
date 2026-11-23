#include "obj_124438.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap hook (also bundled into 0x116030) */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x124438 */
int Obj_124438_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x124438: addi.l on memory = full 32-bit read-modify-write */
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + 0x00010000u);
    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + 0x00006000u);

    /* 0x124448: cmpi.w compares the HIGH word of (0x30d2,A5) */
    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) < 0x0122) {
        /* 0x124430 */
        wr32(m, obj + 0x00u, 0x00124438u);
        return 0;
    }

    /* 0x124450: bfset (0x30ee,A5){0xf:1} */
    wr16(m, F3_A5 + 0x30EEu, (uint16_t)(rd16(m, F3_A5 + 0x30EEu) | 0x8000u));
    wr16(m, obj + 0x06u, 0x0050u);

    /* 0x12445C: beq (branch on the flags still standing from move.w #0x50) */
    int16_t s50 = (int16_t)0x0050;
    if (s50 != 0) {
        /* 0x12445E */
        wr32(m, obj + 0x00u, 0x00124466u);
        return 0;
    }

    /* 0x12446C */
    wr8(m, F3_A5 + 0x30F3u, 0u);                /* sf (0x30f3,A5) */
    int16_t cmd = sub_13F238(m, 0x0013);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
    /* 0x124484 */
    wr8(m, obj + 0x3Au, 0u);                    /* sf (0x3a,A0) */
    /* 0x124488 */
    wr16(m, obj + 0x06u, 0x0003u);
    /* 0x12448E: bfset (0x30ee,A5){0:1} */
    wr16(m, F3_A5 + 0x30EEu, (uint16_t)(rd16(m, F3_A5 + 0x30EEu) | 0x0001u));
    wr16(m, obj + 0x3Eu, 0x001Eu);
    /* 0x12449A */
    wr32(m, obj + 0x00u, 0x001244A2u);
    return 0;
}
