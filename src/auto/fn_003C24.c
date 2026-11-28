/* ROM 0x003C24: register-level wrapper of Sound_StopAll
 * (src/audio/sound_cmd.c) for its capture.
 * movem.l d0-d4/a0-a2,-(a7) ... movem.l (a7)+,d0-d4/a0-a2 restores every
 * register the routine uses, and it takes no arguments: D1 (ring index) is
 * loaded from DPRAM 0xC00480 by Sound_StopAll itself. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003C24(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    Sound_StopAll(m);
    return 0;                                     /* movem restores D0-D4/A0-A2 */
}
