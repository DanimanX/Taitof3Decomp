/* ROM 0x000ECA: register-level wrapper of Dev_StepChannel
 * (src/kernel/devboard.c) for its capture.
 *   000ECA bsr.w  $f04            ; phase timer; d1 = io cur byte, d2 = bit
 *   000ECE btst.l d2,d3 ; beq $f02
 *   000ED2 addq.b #$1,$1(a0) ; jsr $b76 ; 000EDA addq.w #$1,(a4)
 *   000EDC move.w (a2),d0 ; cmp.w (a4),d0 ; bne $f02
 *   000EE2 clr.w  (a4) ; flags ; 000EEE move.w -$7f70(a5),d0
 *   000EF2 add.w  $2(a2),d0 ; cmpi #$9,d0 ; ble $efe ; 000EFC moveq #$9,d0
 *   000EFE move.w d0,-$7f70(a5) ; 000F02 rts
 * d0 is the only register the body writes, and only on the prescale-match path
 * (0x000EEE..0x000EFC): the credit word it just stored, or a full-long 9 when
 * the sum clamps.  Every captured pair has d3 = 0 (btst.l d2,d3 clear) and
 * leaves the register file untouched, SP aside.
 * The take test is detected by the accept counter at (a0)+1, which 0x000ED2 is
 * the only writer of on this path.
 * The masks are the caller's: d1 (0x000F04 btst.l d2,d1) and d3 (0x000ECE
 * btst.l d2,d3), taken from the entry register file. */
#include "fn_auto.h"
#include "../kernel/devboard.h"

int Fn_000ECA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0], a2 = r->a[2], a4 = r->a[4];
    uint8_t accept = rd8(m, a0 + 1u);
    uint16_t credit = rd16(m, DEV_CREDIT);         /* 0x000EEE -$7f70(a5) */
    uint16_t add = rd16(m, a2 + 2u);               /* 0x000EF2 $2(a2) */

    if (Dev_StepChannel(m, a0, a2, a4, (int)(r->d[2] & 31u),   /* btst.l: d2 mod 32 */
                        (uint8_t)r->d[1], (uint8_t)r->d[3]) < 0)
        return -1;                                 /* 0x000F14 bhi -> 0x000F3E */

    if (rd8(m, a0 + 1u) != accept) {               /* 0x000ED2 ran: prescale path */
        uint32_t sum = (uint32_t)credit + add;
        /* 0x000EF6 cmpi.w #$9: ble keeps the word (move.w), else moveq #$9
         * rewrites the whole long. */
        r->d[0] = sum > 9u ? 9u : ((r->d[0] & 0xFFFF0000u) | sum);
    }
    return 0;                                      /* 0x000F02 rts */
}
