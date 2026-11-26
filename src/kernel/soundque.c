/* Darius Gaiden (Taito F3) — sound command queue producer.
 * ROM: 0x000BF6  Sound_PushCmd
 *
 *   w = A5-0x7F7E (0x400082); read = A5-0x7F80 (0x400080)
 *   if (w == read) return 0;                      ; queue full
 *   D0.w = ring[A5-0x8000 (0x400000) + w];        ; move.b (a0,d0.w),D0.b
 *   Sound_Enqueue(0x0200 | D0.w << 1);            ; push, jsr 0x000910
 *   w = (w + 1) & 0x7F;  A5-0x7F7E = w;  return 1;
 *
 * 0x000910 is the MMIO half: it takes the word on the stack and bit-shifts it
 * into the shadow byte at A5-0x5DCA (0x402236), writing that byte to the
 * sound port 0x4A0013 once per loop, 10 times:
 *     D0 = cmd >> 1 (16-bit);  repeat 10:
 *       bit7..1 of the shadow = D0 bits 8..2 (first pass only), bit0 = D0&1
 *       write; then shadow bit1 = 0, write; shadow bit1 = 1, write
 *     D0 = D0 >> 1 (lsr.w fills bit15, not bit0)
 * The port is write-only and lives outside the RAM captures, so it is kept in
 * a static shadow plus a frame log; 0x402236 itself is modelled exactly.
 * 0x000C8A is the consumer (advances the read index, stops once it catches the
 * write index).
 */
#include "soundque.h"

#define SQ_STACK_CMDLOC 0x0200u /* as pushed by 0x000BF6 into 0x402236 byte 1 */

static uint8_t sq_port;          /* last byte written to 0x4A0013 */
static uint8_t sq_log[4096];     /* every frame written, in order */
static int sq_log_len;

void Sound_SerialReset(void) { sq_port = 0; sq_log_len = 0; }
const uint8_t *Sound_SerialLog(int *len) { *len = sq_log_len; return sq_log; }

void Sound_EnqueueExact(const f3mem_t *m, uint16_t cmd)
{
    uint8_t sh = rd8(m, SQ_SHADOW);
    uint16_t d0 = cmd;
    int d1 = 9;
    do {
        int carry = d0 & 1;
        d0 >>= 1;
        if (carry) { sh |= 0x01u; } else { sh &= (uint8_t)~0x01u; }
        sq_port = sh; if (sq_log_len < (int)sizeof sq_log) sq_log[sq_log_len++] = sh;
        sh &= (uint8_t)~0x02u;
        sq_port = sh; if (sq_log_len < (int)sizeof sq_log) sq_log[sq_log_len++] = sh;
        sh |= 0x02u;
        sq_port = sh; if (sq_log_len < (int)sizeof sq_log) sq_log[sq_log_len++] = sh;
    } while (d1--);
    wr8(m, SQ_SHADOW, sh);
}

int Sound_PushCmd(const f3mem_t *m)
{
    uint16_t w = rd16(m, SQ_WRITE_IDX);
    if (w == rd16(m, SQ_READ_IDX)) return 0;
    uint16_t d0 = rd8(m, SQ_RING_BASE + w);
    Sound_EnqueueExact(m, (uint16_t)(SQ_STACK_CMDLOC | (uint16_t)(d0 << 1)));
    w = (uint16_t)(w + 1u);
    if (w == 0x80u) w = 0;
    wr16(m, SQ_WRITE_IDX, w);
    return 1;
}
