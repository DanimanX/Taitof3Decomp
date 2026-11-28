/* ROM 0x001A78: register-level wrapper of IO_PollInputs (src/io/input.c) for
 * its capture.
 *   001A78 moveq #0,D0 ; ori.w #$700,sr ; reads the 0x20-byte port image at
 *   0x4A0000, shifts prev := cur (0x001A90..0x001AAE), packs the image into
 *   io_ch[0..5] at 0x40221C, writes 0 to 0x4A0000 and pushes the 0x402234 /
 *   0x402235 shadows to 0x4A0004 / 0x4A0014, then runs the edge detect 0x001B8E.
 * The port image is MMIO and no RAM dump holds it, so the reads go through the
 * memory model: work/ram/fnk_001A78 was taken with F3_PORTS=1, whose
 * <prefix>_ports.tsv test_fn installs with f3_set_mmio_ports() for the pair it
 * replays. Without such a capture the 0x4A0000 reads return 0, i.e. the old
 * NULL-port behaviour, which only matches a frame with every input released.
 * D0/D1 are scratch. 0x001B8E's last block is channel 5:
 *   001C10 move.b 0x402232,D1b  (the prev just shifted = the OLD cur of ch5)
 *   001C14 move.b 0x402230,D0b  (the new cur)
 *   001C1C not.b D0b ; or.b D1b,D0b ; 001C24 not.b D1b
 * so D0 = 0x000000FF at the rts (moveq #0,D0 at the entry zeroed the whole
 * register and every later D0 op is a byte op), and D1 keeps its entry upper
 * 24 bits - the captured calls enter with 0x0000000A or 0x0000FFFF. */
#include "fn_auto.h"
#include "../io/input.h"

#define IO_CH5_CUR (IO_BASE + 4u * 5u) /* 0x402230 */

int Fn_001A78(const f3mem_t *m, f3regs_t *r)
{
    uint8_t ports[IO_PORT_BYTES];

    for (unsigned i = 0; i < IO_PORT_BYTES; i++)   /* 0x001AB4..0x001B80 */
        ports[i] = rd8(m, IO_POLL_BASE + i);
    uint8_t old_cur5 = rd8(m, IO_CH5_CUR);         /* becomes ch5.prev at 0x001A90 */

    IO_PollInputs(m, ports);                       /* 0x001A78..0x001B8E */

    uint8_t cur5 = rd8(m, IO_CH5_CUR);             /* ch5 packed at 0x001B2E..0x001B4E */
    r->d[0] = (uint32_t)(uint8_t)((uint8_t)~cur5 | old_cur5); /* 0x001C1C/0x001C1E */
    r->d[1] = (r->d[1] & 0xFFFFFF00u) | (uint8_t)~old_cur5;   /* 0x001C10/0x001C24 */
    r->a[7] += 4u;                                 /* 0x001B8C rts */
    return 0;
}
