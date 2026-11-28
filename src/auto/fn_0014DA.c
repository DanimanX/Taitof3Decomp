/* ROM 0x0014DA: register-level wrapper of Kernel_SoundBank0_0014DA /
 * Kernel_SoundBank1_001504 (src/kernel/fn_010420.c) for its capture.
 *   0014DA moveq #$0,d0 ; bsr.w $14e8     ; bank 0, block index 0
 *   0014E0 moveq #$0,d0 ; bsr.w $1504     ; bank 1, block index 0
 *   0014E6 rts
 * Both callees end in the 0x001520 relay, so the register file at the rts is
 * the second block's relay tail (Kernel_SoundRelay returns it). D0/D1 are the
 * entry values only if that block is negative, which the capture's block is
 * not. D2 is not among the registers the ROM writes here. */
#include "fn_auto.h"
#include "../kernel/fn_010420.h"

int Fn_0014DA(const f3mem_t *m, f3regs_t *r)
{
    snd_relay_t rg;

    Kernel_SoundBank0_0014DA(m);                    /* 0x0014DC bsr.w $14e8 */
    rg.d0 = 0u;                                     /* 0x0014E0 moveq #$0,d0 */
    rg.d1 = r->d[1];                                /* untouched by bank 0 here */
    Kernel_SoundRelay(m, 0x000168C0u, 0x0001FB40u, 0u, &rg);  /* bsr.w $1504 */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | (rg.d0 & 0xFFFFu);
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (rg.d1 & 0xFFFFu);
    r->a[0] = rg.a0;
    r->a[1] = rg.a1;
    r->a[2] = rg.a2;
    r->d[3] = rg.d3;
    return 0;                                       /* 0x0014E6 rts */
}
