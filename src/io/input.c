/* Darius Gaiden (Taito F3) — input sampling and edge detection.
 * ROM: 0x001A78  IO_PollInputs
 * ROM: 0x001B8E  IO_EdgeDetect (unrolled x6 in ROM)
 *
 * 0x001A78 clobbers D0,D1 only and touches Work RAM at 0x40221C..0x402235:
 *   moveq #0,D0 ; move.b D0,0x4A0000          ; coin/aux latch reset (MMIO)
 *   move.b A5-0x5DCC,0x4A0004                 ; shadows -> MMIO
 *   move.b A5-0x5DCB,0x4A0014
 *   prev[i] = cur[i] for i = 5..0             ; six move.b -0x5DE4,-0x5DE2 ...
 *   cur[0] = ports[3].hi<<4 | ports[7].lo | ports[2].bit4<<3
 *   cur[1] = ports[3].hi<<4 | ports[7].hi   | ports[2].bit5<<3
 *   cur[2] = ports[2].hi<<4 | ports[7].lo   | ports[2].bit6<<2
 *   cur[3] = ports[2].hi<<4 | ports[7].hi   | ports[2].bit7
 *   cur[4] = ports[1].hi<<4 | ports[2].lo
 *   cur[5] = ports[0].lo    | ports[1].bit1<<3
 *   jsr 0x001B8E
 *   move.b D0b is a byte store, so D0 keeps the 0x..FE of the last pack.
 *
 * 0x001B8E per channel (prev in D1, cur in D0):
 *   move.b D0,pressed ; not.b D0 ; or.b D1,D0 ; move.b D0,released
 *   not.b  D1 ; or.b D1,pressed
 * Note the order: 0x001B8E runs at the *end* of 0x001A78, after cur->prev, so
 * the prev it reads is the byte just shifted there (the previous frame's cur).
 * pressed/released must therefore be computed from that shifted prev, not from
 * the freshly stored cur.
 */
#include "input.h"

io_channel_t io_ch[IO_NUM_CHANNELS];

void IO_EdgeDetectExactFrom(const f3mem_t *m, int prev_shifted)
{
    for (int i = 0; i < IO_NUM_CHANNELS; i++) {
        uint32_t a = IO_BASE + 4u * (uint32_t)i;
        uint8_t cur = rd8(m, a);
        if (!prev_shifted) wr8(m, a + 2u, cur); /* no shift yet: prev := cur */
        uint8_t prev = rd8(m, a + 2u);           /* previous frame's cur */
        wr8(m, a + 1u, (uint8_t)(cur | (uint8_t)~prev));         /* pressed  */
        wr8(m, a + 3u, (uint8_t)((uint8_t)~cur | prev));         /* released */
    }
}

void IO_EdgeDetectExact(const f3mem_t *m)
{
    IO_EdgeDetectExactFrom(m, 0);
}

void IO_EdgeDetect(void)
{
    for (int i = 0; i < IO_NUM_CHANNELS; i++) {
        io_channel_t *c = &io_ch[i];
        uint8_t cur = c->cur, prev = c->prev;
        c->released = (uint8_t)(~cur | prev);
        c->pressed  = (uint8_t)(cur | ~prev);
    }
}

void IO_PollPack(const uint8_t ports[8], uint8_t ch[6])
{
    const uint8_t p0 = ports[0], p1 = ports[1], p2 = ports[2], p3 = ports[3], p7 = ports[7];
    ch[0] = (uint8_t)((p3 << 4) & 0x70u) | (uint8_t)(p7 & 0x0Fu) | (uint8_t)((p2 & 0x10u) << 3);
    ch[1] = (uint8_t)(p3 & 0x70u) | (uint8_t)(p7 >> 4) | (uint8_t)((p2 & 0x20u) << 2);
    ch[2] = (uint8_t)((p2 << 4) & 0x70u) | (uint8_t)(p7 & 0x0Fu) | (uint8_t)((p2 & 0x40u) << 1);
    ch[3] = (uint8_t)(p2 & 0x70u) | (uint8_t)(p7 >> 4) | (uint8_t)(p2 & 0x80u);
    ch[4] = (uint8_t)(p1 & 0xF0u) | (uint8_t)(p2 & 0x0Fu);
    ch[5] = (uint8_t)(p0 & 0x0Fu) | (uint8_t)((p1 & 0x02u) << 3);
}

void IO_PollInputs(const f3mem_t *m, const uint8_t *ports)
{
    static const uint8_t zero[8];
    if (!ports) ports = zero;

    /* 0x001A80/0x001A88 push the two hardware shadows to the 0x4A0004 /
     * 0x4A0014 ports; they read the RAM shadow bytes but never write them, and
     * 0x4A0004 is write-only in this model (the earlier self-store to
     * 0x402234 was not in the ROM: D0 is still 0 there). */
    for (int i = IO_NUM_CHANNELS - 1; i >= 0; i--)
        wr8(m, IO_BASE + 4u * (uint32_t)i + 2u, rd8(m, IO_BASE + 4u * (uint32_t)i));

    uint8_t ch[IO_NUM_CHANNELS];
    IO_PollPack(ports, ch);
    for (int i = 0; i < IO_NUM_CHANNELS; i++) wr8(m, IO_BASE + 4u * (uint32_t)i, ch[i]);

    IO_EdgeDetectExactFrom(m, 1); /* 0x001B8E reads the prev shifted above */
}
