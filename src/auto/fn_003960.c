/* ROM 0x003960: register-level wrapper of Sound_ResetDpram
 * (src/audio/sound_cmd.c) for its capture.
 * The body is bracketed by movem.l d0-d7/a0-a6,-(a7) / movem.l (a7)+,d0-d7/a0-a6,
 * so every register is restored at the rts and only SP moves (which the
 * harness leaves to the caller). D0/A0 are scratch inside the saved window.
 * The 1 KiB DPRAM clear and the two sound-reset MMIO writes are invisible to
 * the RAM compare; the A5-0x1448 / A5-0x1444 stores are Work RAM and are the
 * only compared effect, and Sound_ResetDpram performs them. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003960(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    Sound_ResetDpram(m);
    return 0;
}
