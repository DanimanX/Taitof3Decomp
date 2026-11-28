/* ROM 0x001A78: register-level wrapper of IO_PollInputs (src/io/input.c) for
 * its capture.
 *   001A78 ori.w #$700,sr ; reads the 0x20-byte port image at 0x4A0000, packs
 *   it into io_ch[0..5] at 0x40221C, writes 0 to 0x4A0000 / the 0x402234 and
 *   0x402235 shadows, then runs the edge detect 0x001B8E.
 * The port image itself is MMIO: work/ram/fn_001A78 holds Work RAM / sprite /
 * gfx only, so the wrapper has to pass NULL (all-zero ports) and the produced
 * cur/pressed/released bytes can only match the frames whose input was released.
 * D0/D1 are scratch (the last unrolled edge-detect block leaves the released
 * mask and its complement in their low bytes). */
#include <stddef.h>
#include "fn_auto.h"
#include "../io/input.h"

int Fn_001A78(const f3mem_t *m, f3regs_t *r)
{
    IO_PollInputs(m, NULL);                    /* 0x001A78..0x001B8E */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | 0xFFu;
    r->d[1] = (r->d[1] & 0xFFFFFF00u) | 0x00u;
    return 0;                                  /* rts of 0x001B8E's caller */
}
