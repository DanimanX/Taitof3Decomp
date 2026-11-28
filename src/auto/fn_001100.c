/* ROM 0x001100: register-level wrapper of Sprite_InitHeaders
 * (src/video/sprite_init.c) for its capture.
 *   001100 ori.w  #$100,d0 ; move.w d0,(a2+$a) / +$800a / -$ca4(a5)
 *   001114 lea $603ff0,a0 ; 001148 lea $60bff0,a0 ; headers ; 00117C rts
 * D0 is the only data register written (word form, so the entry's upper half
 * survives) and A0 ends on 0x60BFF0.  A1/A2 are inputs, read but never written;
 * this entry's capture holds A1 = 0x600020, A2 = 0x600000. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_001100(const f3mem_t *m, f3regs_t *r)
{
    Sprite_InitHeaders(m, (uint16_t)(r->d[0] | 0x0100u)); /* 0x001100..0x00117C */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)(r->d[0] | 0x0100u);
    r->a[0] = 0x0060BFF0u;                                /* 0x001148 lea $60bff0,a0 */
    return 0;                                             /* 0x00117C rts */
}
