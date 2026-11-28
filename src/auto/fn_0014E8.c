/* ROM 0x0014E8: register-level wrapper of the bank-0 entry of the 0x001520
 * block relay (src/kernel/fn_010420.c, Kernel_SoundRelay) for its capture.
 *   0014E8 lea $16890,a0 ; 0014EE lea $16900,a1
 *   0014F4 andi.l #$ff,d0 ; 0014FA divu.w #$6,d0 ; 0014FE swap d0
 *   001500 bra.w $1520     ; relay: movea.l (a0,d0.w*8),a2 ; move.l 4(...),d3
 * The index is the divu remainder, so `swap` leaves d0 = rem<<16 | quotient only
 * on the negative-block path; the relay tail (d0/d1/a2/a0/d3) is what
 * Kernel_SoundRelay reports. */
#include "fn_auto.h"
#include "../kernel/fn_010420.h"

#define BANK0_TABLE 0x00016890u
#define BANK0_SRC   0x00016900u

int Fn_0014E8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t byte = r->d[0] & 0xFFu;                                  /* 0x0014F4 andi.l */
    uint16_t quotient = (uint16_t)(byte / 6u), rem = (uint16_t)(byte % 6u); /* 0x0014FA */
    snd_relay_t rg;

    rg.d0 = r->d[0];
    rg.d1 = r->d[1];
    Kernel_SoundRelay(m, BANK0_TABLE, BANK0_SRC, (uint8_t)rem, &rg);  /* 0x001520 */

    r->a[1] = BANK0_SRC;                                              /* 0x0014EE lea */
    if (rg.a0 == BANK0_TABLE) {                                       /* 0x001528 bmi */
        r->d[0] = ((uint32_t)rem << 16) | quotient;                   /* 0x0014FE swap d0 */
        r->d[3] = rg.d3;
    } else {
        r->d[0] = (r->d[0] & 0xFFFF0000u) | (rg.d0 & 0xFFFFu);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | (rg.d1 & 0xFFFFu);
        r->d[3] = rg.d3;
    }
    r->a[0] = rg.a0;
    r->a[2] = rg.a2;
    return 0;                                                         /* 0x00154C rts */
}
