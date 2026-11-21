#ifndef TAITOF3_IO_INPUT_H
#define TAITOF3_IO_INPUT_H
#include <stdint.h>

/* One input channel, active-low bits (0 = pressed). 0x40221C + 4*n */
typedef struct {
    uint8_t cur;      /* +0 sampled this frame */
    uint8_t pressed;  /* +1 0 where cur=0 && prev=1 (new press) */
    uint8_t prev;     /* +2 previous frame */
    uint8_t released; /* +3 0 where cur=1 && prev=0 (new release) */
} io_channel_t;

#define IO_NUM_CHANNELS 6
extern io_channel_t io_ch[IO_NUM_CHANNELS]; /* 0x40221C */

void IO_EdgeDetect(void); /* ROM 0x001B8E */

#endif
