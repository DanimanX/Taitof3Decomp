#ifndef TAITOF3_KERNEL_DEVBOARD_H
#define TAITOF3_KERNEL_DEVBOARD_H
#include "../common/f3mem.h"

/* Coin/service dev-board credit stepper. Layout (docs/KERNEL.md, task0 notes):
 * ch0   A5-0x7F6A (0x400096)  state.b  +0 hold/error timer, +1 accepted
 *                             presses, +2 service-button hold blinker
 * ch1   A5-0x7F66 (0x40009A)  same
 * win0  A5-0x7F5C (0x4000A4)  2 words +0 prescale, +2 credit-per-hit
 * win1  A5-0x7F58 (0x4000A8)  same, +0 reached through the A5-0x7F62 pointer
 * credit A5-0x7F70 (0x400090)  capped at 9
 * frame  A5-0x7F54 (0x4000AC), queue flags A5-0x7F51 /-0x7F52 (0x4000AF/AE) */
#define DEV_CH0      (F3_A5 - 0x7F6Au) /* 0x400096 */
#define DEV_CH1      (F3_A5 - 0x7F66u) /* 0x40009A */
#define DEV_WIN0     (F3_A5 - 0x7F5Cu) /* 0x4000A4 */
#define DEV_WIN1     (F3_A5 - 0x7F58u) /* 0x4000A8 */
#define DEV_WIN1_PTR (F3_A5 - 0x7F62u) /* 0x40009E */
#define DEV_CREDIT   (F3_A5 - 0x7F70u) /* 0x400090 */
#define DEV_ENABLE   (F3_A5 - 0x5DC7u) /* 0x402239 */

/* ROM 0x000ECA, one coin channel: phase timer (0x000F04, tests bit d2 of d1),
 * then if d3 has bit d2: count++, (a4)++ and on (a2)==(a4) reset it, signal the
 * sound queue and add (a2+2) credits (clamped at 9). d1/d3 are the caller's
 * masks: d1 = io_ch[1].cur, d3 = ~io_ch[1].released (0x000E18/0x000E1C).
 * Returns 0, or -1 when the phase timer took the 0x000F3E error path. */
int Dev_StepChannel(const f3mem_t *m, uint32_t chan, uint32_t a2, uint32_t a4, int d2,
                    uint8_t d1, uint8_t d3);

/* ROM 0x000E00  DevBoard_ServiceTask. 0x000E00 tests bit0 of io_ch[1].cur
 * (0x402220) and jumps to 0x000F54 when it is clear, i.e. the body runs while
 * channel 1's bit0 is set. The 0x000E14 call to 0x000FA8 (pending coin-sound
 * queue service) is modelled by Fn_000FA8 (src/auto/fn_000FA8.c), not here.
 * Returns 0 when the 0x000F54 / 0x000F3E paths were taken, else 1. */
int DevBoard_ServiceTask(const f3mem_t *m);

#endif
