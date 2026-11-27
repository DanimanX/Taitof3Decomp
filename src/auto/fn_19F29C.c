/* ROM 0x19F29C: stamp one character into the "AL: ..." debug string at
 * 0x40CBC4. Reads the letter index from the word at A5+0x32DC, looks it up in
 * the stride-2 byte table at 0x19F2C6 (0x41..0x55, 0x5C, 0x56..0x5B), and
 * stores it at 0x40CBC7 + (word at A5+0x32DE - 1). D0/D1/A1 are saved and
 * restored, so no register changes at rts. */
#include "fn_auto.h"

int Fn_19F29C(const f3mem_t *m, f3regs_t *r)
{
    (void)r;

    uint32_t tbl = 0x0019F2C6u;                                  /* 0x19F2A0 lea (0x24,PC),A1 (PC=0x19F2A2) */
    int32_t idx = (int16_t)rd16(m, F3_A5 + 0x32DCu);              /* 0x19F2A6 */
    uint8_t ch = rd8(m, tbl + (uint32_t)(idx * 2));               /* 0x19F2AA move.b (0,A1,D0.w*2) */
    int32_t col = (int16_t)(uint16_t)(rd16(m, F3_A5 + 0x32DEu) - 1u); /* 0x19F2B4/0x19F2B8 */
    uint32_t dst = 0x0040CBC4u + 3u + (uint32_t)col;              /* 0x19F2AE/0x19F2BA */

    wr8(m, dst, ch);                                             /* 0x19F2BE */
    return 0;
}
