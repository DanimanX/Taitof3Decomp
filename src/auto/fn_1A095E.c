/* ROM 0x1A095E: entry segment of the test-mode "HI-SCORE RESET" screen
 * (src/test/seg_1A095E.c), stopped exactly where the screen first yields.
 *
 * Like 0x1A043C the screen never returns: the YES/NO frame loop ends in
 * `jsr 0x001F62` (Sys_Yield) at 0x1A09A6, and the "yes" arm at 0x1A0A2C runs
 * 0x00509A/0x004DEE, whose serial-EEPROM write-back enters the 0x01014E reset
 * loop. So the capture stops AT 0x1A09A6 (F3_EXIT=1A09A6,
 * F3_INJECT_ABORT=1): the post dump is the state after Gfx_ClearAllLayers, the
 * settings load 0x4D06 and the two strings ("HI-SCORE RESET" at 0x61C09A, "ARE
 * YOU SURE?" at 0x61C41C).
 *
 * Verified bit-exact on work/ram/fnt_1A095E (30 pairs, entry 0x1A095E ->
 * 0x1A09A6). */
#include "../kernel/seg.h"

int Fn_1A095E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t pc = Seg_1A095E(m, r, 0x001A095Eu);
    return pc == 0x001A09A6u ? 0 : -1;   /* 0x1A09A6 jsr 0x001F62 */
}
