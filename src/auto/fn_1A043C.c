/* ROM 0x1A043C: entry segment of the test-mode "SOUND TEST" screen
 * (src/test/seg_1A043C.c), stopped exactly where the screen first yields.
 *
 * The screen never returns to its caller: its frame loop ends in
 * `jsr 0x001F62` (Sys_Yield) at 0x1A04D4, and the only rts (0x1A0940) lies
 * behind the 0x1A091C yield and the 0x004DEE serial-EEPROM write-back that
 * enters the 0x01014E reset loop. So the capture stops AT that jsr
 * (F3_EXIT=1A04D4, F3_INJECT_ABORT=1): the post dump is the state the entry
 * segment left, before the call gate builds the TRAP #5 frame.
 *
 * The segment model is register- and RAM-accurate for that point (it runs
 * 0x1A043C..0x1A04D4: link/movem frame, Gfx_ClearAllLayers, the 0x48-long
 * character block, sound 0, settings 0x4D06, the record directory 0x4088, the
 * eight track volumes and the "SOUND TEST" title). Verified bit-exact on
 * work/ram/fnt_1A043C (30 pairs, entry 0x1A043C -> 0x1A04D4). */
#include "../kernel/seg.h"

int Fn_1A043C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t pc = Seg_1A043C(m, r, 0x001A043Cu);
    return pc == 0x001A04D4u ? 0 : -1;   /* 0x1A04D4 jsr 0x001F62 */
}
