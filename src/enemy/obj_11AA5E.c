#include "obj_11AA5E.h"
#include "../audio/sound_cmd.h"

/* ROM 0x11AA5E: clear row 0x626200 (0xE8 words of -0x0FF1), stash the
 * 0x1D36 flags byte in +0x34, set bit 1 and clear bit 4 of 0x1D36, sound
 * command 0x2D (through the 0x13F238 remap) and arm the +0x6 timer with 0x20.
 * The `beq` right after the move.w only tests the flags of that move, so the
 * 0x11AAB8 entry (0x11AAB0) is always taken. */
int Obj_11AA5E_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0011AA5Eu);              /* 0x11AA56 */
        return 0;
    }

    uint8_t bias = rd8(m, F3_A5 - 0x5D56u);             /* adda.w (-0x5d56,A5) */
    uint32_t a0 = 0x00626200u + (uint32_t)bias;
    for (uint32_t d7 = 0u; d7 <= 0xE7u; d7++) {         /* dbf D7: 0xE8 words */
        wr16(m, a0, 0xFFF1u);
        a0 += 2u;                                      /* move.w #-0xff1,(A0)+ */
    }

    wr8(m, obj + 0x34u, rd8(m, F3_A5 + 0x1D36u));       /* 0x11AA82 */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x02u)); /* bset.b #1 */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & (uint8_t)~0x10u)); /* bclr.b #4 */

    /* 0x11AA94: move.w #0x2d,-(SP); jsr 0x13F238; jsr 0x39D8 */
    int16_t cmd = 0x002D;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    wr16(m, obj + 0x06u, 0x0020u);                      /* 0x11AAA8 */
    wr32(m, obj + 0x00u, 0x0011AAB8u);                  /* 0x11AAB0 */
    return 0;
}
