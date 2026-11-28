/* ROM 0x001504: register-level wrapper of the bank-1 entry of the 0x001520
 * block relay (src/kernel/fn_010420.c, Kernel_SoundRelay) for its capture.
 *   001504 lea $168c0,a0 ; 00150A lea $1fb40,a1
 *   001510 andi.l #$ff,d0 ; 001516 divu.w #$8,d0 ; 00151A swap d0
 *   00151C bra.w $1520
 * Index = the divu remainder (word); `swap d0` only survives on the
 * negative-block path. The tail comes from Kernel_SoundRelay. */
#include "fn_auto.h"
#include "../kernel/fn_010420.h"

#define BANK1_TABLE 0x000168C0u
#define BANK1_SRC   0x0001FB40u

int Fn_001504(const f3mem_t *m, f3regs_t *r)
{
    uint32_t byte = r->d[0] & 0xFFu;                                  /* 0x001510 andi.l */
    uint16_t quotient = (uint16_t)(byte / 8u), rem = (uint16_t)(byte % 8u); /* 0x001516 */
    snd_relay_t rg;

    rg.d0 = r->d[0];
    rg.d1 = r->d[1];
    Kernel_SoundRelay(m, BANK1_TABLE, BANK1_SRC, (uint8_t)rem, &rg);  /* 0x001520 */

    r->a[1] = BANK1_SRC;                                              /* 0x00150A lea */
    if (rg.a0 == BANK1_TABLE) {                                       /* 0x001528 bmi */
        r->d[0] = ((uint32_t)rem << 16) | quotient;                   /* 0x00151A swap d0 */
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
