/* ROM 0x003CBA: register-level wrapper of Sound_SetVolume
 * (src/audio/sound_cmd.c) for its capture.
 * movem.l a0,-(a7) (4 bytes) brackets the body, so every register is unchanged
 * at the rts; A0 is only the 0xC007F8 destination base. The four byte
 * arguments are read at 0x9/0xb/0xd/0xf(a7) with a7 = entry SP-4, i.e. the low
 * bytes of the four words the caller pushes at entry SP+4, SP+6, SP+8, SP+0xA:
 * entry SP+5, +7, +9, +0xB. Only DPRAM is written, so no compared RAM. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003CBA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint8_t a = rd8(m, sp + 5u);                  /* 0x003CC4 move.b 0x9(a7),(a0) */
    uint8_t b = rd8(m, sp + 7u);                  /* 0x003CC8 move.b 0xb(a7),0x1(a0) */
    uint8_t c = rd8(m, sp + 9u);                  /* 0x003CCE move.b 0xd(a7),0x2(a0) */
    uint8_t d = rd8(m, sp + 0x0Bu);               /* 0x003CD4 move.b 0xf(a7),0x3(a0) */

    Sound_SetVolume(m, a, b, c, d);
    return 0;                                     /* movem.l (a7)+,a0 */
}
