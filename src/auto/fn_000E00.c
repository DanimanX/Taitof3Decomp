/* ROM 0x000E00: register-level wrapper of DevBoard_ServiceTask
 * (src/kernel/devboard.c) for its capture.
 *   000E00 btst.b #$0,-$5de0(a5)   ; 0x402220 = io_ch[1].cur, active low
 *   000E06 beq.w  $f54             ; switch not sampled pressed -> halt path
 *   000E0A cmpi.w #$9,-$7f70(a5) ; bhi $f3e      (credit > 9 -> error path)
 *   000E14 bsr.w  $fa8             ; queue service (no register output)
 *   000E18 move.b -$5de0(a5),d1    ; d1 = cur byte
 *   000E1C move.b -$5ddd(a5),d3    ; d3 = ~released byte (0x402223)
 *   000E22 moveq  #$4,d2 ... 000EC8 rts
 * The last live register writes before the rts are the channel-1 step
 * (0x000E34..0x000E42: d2=5, a0=0x40009A, a2=0x4000A8, a4=rd32(0x40009E)) and,
 * when the blink runs, the channel-1 blink setup (0x000E90..0x000E9A: a0,
 * a4=0x402234, moveq #$3,d1).  d0 is untouched: the only d0 write is inside
 * Dev_StepChannel's prescale match, covered by Fn_000ECA itself.
 * NB the ROM reads io_ch[1] (0x402220/0x402223); the capture's post D1 =
 * 0x000000FF is that byte, while 0x40221C (io_ch[0].cur) held 0x1F. */
#include "fn_auto.h"
#include "../kernel/devboard.h"

#define DEV_IO1_CUR      (F3_A5 - 0x5DE0u) /* 0x402220 io_ch[1].cur */
#define DEV_IO1_RELEASED (F3_A5 - 0x5DDDu) /* 0x402223 io_ch[1].released */

int Fn_000E00(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = rd32(m, F3_A5 - 0x7F62u);   /* 0x000E3E movea.l -$7f62(a5),a4 */

    if (!DevBoard_ServiceTask(m))             /* 0x000F54 / 0x000F3E halt paths */
        return -1;

    r->d[1] = (r->d[1] & 0xFFFFFF00u) | rd8(m, DEV_IO1_CUR);              /* 0x000E18 */
    r->d[3] = (r->d[3] & 0xFFFFFF00u) | (uint8_t)~rd8(m, DEV_IO1_RELEASED); /* 0x000E1C/20 */
    r->d[2] = 5u;                             /* 0x000E34 moveq #$5,d2 */
    r->a[0] = F3_A5 - 0x7F66u;                /* 0x000E36 lea -$7f66(a5),a0 */
    r->a[2] = F3_A5 - 0x7F58u;                /* 0x000E3A lea -$7f58(a5),a2 */
    r->a[4] = a4;
    /* 0x000E6A tst.b -$5dc7(a5) ; 0x000E86 tst.b $1(a0) with a0 = ch1. */
    if (rd8(m, F3_A5 - 0x5DC7u) && rd8(m, F3_A5 - 0x7F65u)) {
        r->d[1] = 3u;                         /* 0x000E98 moveq #$3,d1 */
        r->a[4] = F3_A5 - 0x5DCCu;            /* 0x000E94 lea -$5dcc(a5),a4 */
    }
    return 0;                                 /* 0x000EC8 rts */
}
