#ifndef TAITOF3_IO_INPUT_H
#define TAITOF3_IO_INPUT_H
#include <stdint.h>
#include "../common/f3mem.h"

/* One input channel, active-low bits (0 = pressed). 0x40221C + 4*n */
typedef struct {
    uint8_t cur;      /* +0 sampled this frame */
    uint8_t pressed;  /* +1 0 where cur=0 && prev=1 (new press) */
    uint8_t prev;     /* +2 previous frame */
    uint8_t released; /* +3 0 where cur=1 && prev=0 (new release) */
} io_channel_t;

#define IO_NUM_CHANNELS 6
#define IO_BASE 0x40221Cu                /* io_ch[0] */
#define IO_OUT_4A0004 (F3_A5 - 0x5DCCu)  /* 0x402234, shadow of 0x4A0004 */
#define IO_OUT_4A0014 (F3_A5 - 0x5DCBu)  /* 0x402235, shadow of 0x4A0014 */
#define IO_POLL_BASE 0x4A0000u           /* 0x001A78 reads 0x4A0000..0x4A0017 */
extern io_channel_t io_ch[IO_NUM_CHANNELS]; /* 0x40221C */

void IO_EdgeDetect(void);                  /* ROM 0x001B8E, on io_ch[] */
void IO_EdgeDetectExact(const f3mem_t *m); /* ROM 0x001B8E, on 0x40221C */
/* Same, with control over the cur->prev shift: 0x001B8E runs after 0x001A78 has
 * already shifted prev (so pressed/released are computed against the old prev).
 * prev_shifted=0 shifts prev := cur first; 1 uses the prev already in RAM. */
void IO_EdgeDetectExactFrom(const f3mem_t *m, int prev_shifted);

/* Channel packing of 0x001A78 from the 0x20-byte image of 0x4A0000..0x4A001F
 * (bytes 0x00-0x03, 0x07, 0x12 and 0x17 are read). */
#define IO_PORT_BYTES 0x20
void IO_PollPack(const uint8_t ports[IO_PORT_BYTES], uint8_t ch[6]);

/* ROM 0x001A78 IO_PollInputs. ports = the 0x20-byte image of 0x4A0000; NULL = all zero (all zero in a
 * RAM-only capture: the port image is not part of it). Always shifts
 * cur->prev, writes 0 to 0x4A0000 and the two shadows to /4A0004, /4A0014,
 * stores the packed bytes into io_ch[i].cur, then IO_EdgeDetect (001B8E).
 * Clobbers D0,D1. */
void IO_PollInputs(const f3mem_t *m, const uint8_t *ports);

#endif
