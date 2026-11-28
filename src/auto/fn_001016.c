/* ROM 0x001016: register-level wrapper of Coin_CoinsNeeded
 * (src/player/player_task.c) for its capture.
 *   001016 move.w -$7f5e(a5),d0 ; beq $102c        ; d0.w = price word
 *   00101C sub.w  -$7f6e(a5),d0 ; ble $1028
 *   001020 tst.w  -$7f70(a5) ; beq $102a
 *   001022 (fall through to the rts: D0.w = the word difference)
 *   001028 moveq  #$0,d0 ; bra $1032               ; full long cleared
 *   00102C tst.w  -$7f70(a5) ; 001030 seq.b d0     ; low byte only
 *   001032 rts
 * So D0's upper half is the entry's (`move.w` / `sub.w` / `seq.b` all keep it),
 * bits 8-15 come from the price word or from the difference, and only the
 * `moveq #0` path rewrites the whole long.  The C returns just the byte, so the
 * word is rebuilt here from the same three RAM words; the byte itself is the
 * C's return value. */
#include "fn_auto.h"
#include "../player/player_task.h"

int Fn_001016(const f3mem_t *m, f3regs_t *r)
{
    uint16_t price = rd16(m, F3_A5 - 0x7F5Eu);
    uint16_t need = rd16(m, F3_A5 - 0x7F6Eu);
    uint16_t credit = rd16(m, F3_A5 - 0x7F70u);
    uint16_t diff = (uint16_t)(price - need);          /* 0x00101C sub.w */
    uint32_t upper = r->d[0] & 0xFFFF0000u;
    uint8_t byte = Coin_CoinsNeeded(m);

    if (price == 0u)                                   /* 0x00101A beq $102c */
        r->d[0] = upper | (price & 0xFF00u) | byte;    /* 0x001030 seq.b d0 */
    else if ((int16_t)diff > 0 && credit == 0u)        /* 0x001020 ble / 0x001026 beq */
        r->d[0] = upper | diff;                        /* 0x001022..0x00102A */
    else
        r->d[0] = 0u;                                  /* 0x001028 moveq #$0,d0 */
    return 0;                                          /* 0x001032 rts */
}
