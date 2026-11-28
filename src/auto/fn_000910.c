/* ROM 0x000910: the serial half of the sound command push (MMIO), translated
 * at register level for its capture (work/ram/fnk_000910, 9 pairs).
 *
 *   000910  lea.l (-0x5DCA,A5),A0        ; 0x402236 sound-port shadow byte
 *   000914  lea.l 0x4A0013,A1           ; write-only serial port
 *   00091A  move.w (0x4,A7),D0w         ; the word 0x000BF6 pushed
 *   00091E  moveq #0x9,D1
 *   000920  lsr.w #0x1,D0               ; 10 passes of (carry -> shadow bit0,
 *   000922  bcs 0x00092C                ;  write, bit1=0, write, bit1=1, write)
 *   00092C  bset.b #0x0,(A0) ... 0x00093E dbra D1,0x000920
 *   000942  rts
 *
 * The port itself is not in the RAM captures; Sound_EnqueueExact keeps the
 * shadow at 0x402236 exact and logs the writes. Each of the 9 captured calls
 * leaves the shadow at 0x03, and that byte is the only Work RAM change, so the
 * empty body fails all 9. Registers at the rts: A0 and A1 as loaded above,
 * D0w = the argument shifted right 10 times, D1w = -1 from the final dbra
 * (both in their low words, high words kept). */
#include "fn_auto.h"
#include "../kernel/soundque.h"

int Fn_000910(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t cmd = rd16(m, sp + 4u);                       /* 0x00091A */

    Sound_EnqueueExact(m, cmd);                            /* 0x000920..0x00093E */

    r->a[0] = SQ_SHADOW;                                   /* 0x000910 lea */
    r->a[1] = 0x004A0013u;                                 /* 0x000914 lea */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)(cmd >> 10); /* 10x lsr.w #1 */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0xFFFFu;           /* 0x00093E dbra -> -1 */
    r->a[7] = sp + 4u;                                     /* 0x000942 rts */
    return 0;
}
