/* ROM 0x000978: queue a 0xA8-family serial command with one packed number.
 * Reads its stack arguments directly from the (uncopied) frame: the byte
 * command selector at entry SP+4, OR-ed with 0xA8, goes through the serial
 * queue 0x000C8A, the signed word at entry SP+6 is emitted as 2 decimal
 * digits by 0x000C30, and 0xFF terminates the command via 0x000C8A. */
#include "fn_auto.h"
#include "fn_call_frames.h"

int Fn_000978(const f3mem_t *m, f3regs_t *r)
{
    /* ROM 0x000978 link.w A6,#0 -> A6 = entry SP-4, so (0x8,A6) = SP+4 and
     * (0xa,A6) = SP+6 in the caller's frame. */
    uint32_t sp = r->a[7];
    uint8_t cmd = (uint8_t)(rd8(m, sp + 4u) | 0xA8u);          /* 0x00097C..0x000980 */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | cmd;
    r->a[7] -= 2u;                                            /* 0x000984 move.b D0b,-(SP) */
    wr8(m, r->a[7], cmd);
    push_long(m, r, 0x00098Au);                               /* 0x000986 bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                            /* 0x00098A addq.l #2,SP */

    uint32_t val = (uint32_t)(int32_t)(int16_t)rd16(m, sp + 6u); /* 0x00098C..0x000990 */
    r->d[0] = val;
    push_word(m, r, 2);                                       /* 0x000992 move.w #2,-(SP) */
    push_long(m, r, val);                                     /* 0x000996 move.l D0,-(SP) */
    push_long(m, r, 0x00099Cu);                               /* 0x000998 bsr 0x000C30 */
    Fn_000C30(m, r);
    r->a[7] += 6u;                                            /* 0x00099C addq.l #6,SP */

    r->a[7] -= 2u;                                            /* 0x00099E move.b #-0x1,-(SP) */
    wr8(m, r->a[7], 0xFFu);
    push_long(m, r, 0x0009A6u);                               /* 0x0009A2 bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                            /* 0x0009A6 addq.l #2,SP */
    return 0;                                                 /* 0x0009A8 unlk A6 ; rts */
}
