/* ROM 0x003B66: register-level wrapper of Sound_PlayCmdForced
 * (src/audio/sound_cmd.c) for its capture.
 * movem.l d0-d2/a0-a2,-(a7) ... movem.l (a7)+,d0-d2/a0-a2 restores all of
 * D0-D2/A0-A2, so the register file is unchanged at the rts. The movem saves
 * 6 registers (24 bytes = 0x18), so `move.w 0x1c(a7),d0` (0x003B6A) is the
 * command word at entry SP+4; it is a word read and the ROM indexes with
 * d0.w sign-extended, i.e. an int16_t. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003B66(const f3mem_t *m, f3regs_t *r)
{
    int16_t cmd = (int16_t)rd16(m, r->a[7] + 4u); /* 0x003B6A move.w 0x1c(a7),d0 */
    Sound_PlayCmdForced(m, cmd);
    return 0;                                     /* movem restores D0-D2/A0-A2 */
}
