/* ROM 0x001520: register-level wrapper of the sound-bank block relay
 * (src/kernel/fn_010420.c, Kernel_SoundRelay) for its capture.
 *   001520 movea.l (a0,d0.w*8),a2 ; 001524 move.l $4(a0,d0.w),d3 ; 001528 bmi
 *   00152A lea.l $440000,a0 ; block loops ; 00154C rts
 * A0/A1 are the caller's table and source base, D0.w the block index. Every
 * register the relay leaves is what Kernel_SoundRelay reports, including a0
 * = 0x440000 (or the table on the negative path) and d3's dbra word. */
#include "fn_auto.h"
#include "../kernel/fn_010420.h"

int Fn_001520(const f3mem_t *m, f3regs_t *r)
{
    uint16_t idx = (uint16_t)r->d[0];                  /* 0x001520 d0.w = block index */
    snd_relay_t rg = { r->d[0], r->d[1], 0u, 0u, 0u, 0u };

    Kernel_SoundRelay(m, r->a[0], r->a[1], (uint8_t)idx, &rg);

    if (rg.a0 == r->a[0]) {                            /* 0x001528 bmi: no copy */
        r->d[3] = rg.d3;
    } else {
        r->d[0] = (r->d[0] & 0xFFFF0000u) | (rg.d0 & 0xFFFFu);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | (rg.d1 & 0xFFFFu);
        r->d[3] = rg.d3;
    }
    r->a[0] = rg.a0;
    r->a[2] = rg.a2;
    return 0;                                          /* 0x00154C rts */
}
