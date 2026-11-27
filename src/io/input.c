/* Darius Gaiden (Taito F3) — input sampling and edge detection.
 * ROM: 0x001A78  IO_PollInputs
 * ROM: 0x001B8E  IO_EdgeDetect (unrolled x6 in ROM)
 *
 * 0x001A78 clobbers D0,D1 only and touches Work RAM at 0x40221C..0x402235:
 *   moveq #0,D0 ; move.b D0,0x4A0000          ; coin/aux latch reset (MMIO)
 *   move.b A5-0x5DCC,0x4A0004                 ; shadows -> MMIO
 *   move.b A5-0x5DCB,0x4A0014
 *   prev[i] = cur[i] for i = 5..0             ; six move.b -0x5DE4,-0x5DE2 ...
 *   cur[i] at 0x40221C + 4*i, p = port bytes at 0x4A0000 + n (see IO_PollPack):
 *   cur[2] = p[3]<<4 & 0x70 | p[7] & 0x0F | (p[2] & 0x10)<<3      (P1)
 *   cur[3] = p[3] & 0x70    | p[7] >> 4   | (p[2] & 0x20)<<2      (P2)
 *   cur[4] = p[0x12]<<4 & 0x70 | p[0x17] & 0x0F | (p[2] & 0x40)<<1 (P3)
 *   cur[5] = p[0x12] & 0x70 | p[0x17] >> 4 | p[2] & 0x80          (P4)
 *   cur[1] = p[1] & 0xF0    | p[2] & 0x0F
 *   cur[0] = p[0] & 0x0F    | (p[1] & 0x02)<<3
 *   (fixed 2026-11-27 against the VBlank capture: the old table permuted
 *   the channels and read P3/P4 from p[2]/p[7]; all-zero ports hid it)
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

/* 0x001AB4..0x001B84: ch[i] is the byte at IO_BASE + 4*i (0x40221C + 4*i);
 * p = the 0x20-byte port image at 0x4A0000. ROM write order: ch2, ch3, ch4,
 * ch5, ch1, ch0 (no channel reads another, so the order is not visible). */
void IO_PollPack(const uint8_t p[IO_PORT_BYTES], uint8_t ch[6])
{
    ch[2] = (uint8_t)(((p[0x03] << 4) & 0x70u) | (p[0x07] & 0x0Fu) | ((p[0x02] & 0x10u) << 3));
    ch[3] = (uint8_t)((p[0x03] & 0x70u) | (p[0x07] >> 4) | ((p[0x02] & 0x20u) << 2));
    ch[4] = (uint8_t)(((p[0x12] << 4) & 0x70u) | (p[0x17] & 0x0Fu) | ((p[0x02] & 0x40u) << 1));
    ch[5] = (uint8_t)((p[0x12] & 0x70u) | (p[0x17] >> 4) | (p[0x02] & 0x80u));
    ch[1] = (uint8_t)((p[0x01] & 0xF0u) | (p[0x02] & 0x0Fu));
    ch[0] = (uint8_t)((p[0x00] & 0x0Fu) | ((p[0x01] & 0x02u) << 3));
}

void IO_PollInputs(const f3mem_t *m, const uint8_t *ports)
{
    static const uint8_t zero[IO_PORT_BYTES];
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
