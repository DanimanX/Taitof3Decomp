/* Darius Gaiden (Taito F3) — input edge detection.
 * ROM: 0x001B8E  IO_EdgeDetect (unrolled x6 in ROM)
 *
 *   move.b  prev,D1 ; move.b cur,D0
 *   move.b  D0,pressed ; not.b D0 ; or.b D1,D0 ; move.b D0,released
 *   not.b   D1 ; or.b D1,pressed
 */
#include "input.h"

io_channel_t io_ch[IO_NUM_CHANNELS];

void IO_EdgeDetect(void)
{
    for (int i = 0; i < IO_NUM_CHANNELS; i++) {
        io_channel_t *c = &io_ch[i];
        uint8_t cur = c->cur, prev = c->prev;
        c->released = (uint8_t)(~cur | prev);
        c->pressed  = (uint8_t)(cur | ~prev);
    }
}
