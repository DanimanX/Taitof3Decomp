/* ROM 0x001070: register-level wrapper of Sprite_InitBuffers
 * (src/video/sprite_init.c) for its capture.
 *   001070 lea $600010,a0 ; 001076 lea $600020,a1 ; 00107C lea $600000,a2
 *   ... headers, 0010A2 move.w -$5d58(a5),d0 ; andi.w #$1,d0 ; ... ror.w #$3,d0
 *   001100 ori.w #$100,d0 ; move.w d0,(a2+$a) / +$800a / -$ca4(a5)
 *   001114/001148 lea $603ff0 / $60bff0,a0 ; headers ; 00117C rts
 * So at the rts: A2 = 0x600000 (0x00107C), A1 = 0x600020 (0x001076), A0 =
 * 0x60BFF0 (the second header's lea), and D0.w = the flip value (bit0 of
 * 0x4022A8, `move.w` keeps the entry's upper half) rotated right by 3 - 0 or
 * 0x2000 - then or'ed with 0x0100.  Everything else is untouched. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_001070(const f3mem_t *m, f3regs_t *r)
{
    uint16_t flip;

    Sprite_InitBuffers(m);                             /* 0x001070..0x0010FE + 0x001100 */

    flip = (uint16_t)(rd16(m, F3_A5 - 0x5D58u) & 1u);   /* 0x0010A2/0010B6 */
    flip = (uint16_t)((flip << 13) | (flip >> 3));      /* 0x0010FE ror.w #$3 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)(flip | 0x0100u); /* 0x001100 */
    r->a[0] = 0x0060BFF0u;                              /* 0x001148 lea $60bff0,a0 */
    r->a[1] = 0x00600020u;                              /* 0x001076 lea $600020,a1 */
    r->a[2] = 0x00600000u;                              /* 0x00107C lea $600000,a2 */
    return 0;                                           /* 0x00117C rts */
}
