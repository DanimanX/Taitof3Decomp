#ifndef TAITOF3_KERNEL_SOUNDQUE_H
#define TAITOF3_KERNEL_SOUNDQUE_H
#include <stdint.h>
#include "../common/f3mem.h"

#define SQ_RING_BASE  (F3_A5 - 0x8000u) /* 0x400000, 128-byte ring  */
#define SQ_READ_IDX   (F3_A5 - 0x7F80u) /* 0x400080                */
#define SQ_WRITE_IDX  (F3_A5 - 0x7F7Eu) /* 0x400082                */
#define SQ_SHADOW     (F3_A5 - 0x5DCAu) /* 0x402236 sound port shadow */

/* 0x4A0013 is write-only; 0x000910 pushes 10 frames of (bit1,bit0) =
 * (0,bit1 of cmd) x9 then (1,bit0 of cmd). Recorded here, one byte per frame,
 * plus the last value written, so a test can predict 0x402236 exactly. */
void Sound_SerialReset(void);
const uint8_t *Sound_SerialLog(int *len);

void Sound_EnqueueExact(const f3mem_t *m, uint16_t cmd); /* ROM 0x000910 (MMIO half) */
int  Sound_PushCmd(const f3mem_t *m);                    /* ROM 0x000BF6 */

#endif
