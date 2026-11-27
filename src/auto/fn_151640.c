/* ROM 0x151640: copy 16 palette longs from ROM 0x14E19A to 0x40B42E and
 * queue them as a 0x10-entry upload to palette RAM 0x0B80. */
#include "fn_auto.h"
#include "../video/palette.h"

int Fn_151640(const f3mem_t *m, f3regs_t *r)
{
    /* 0x151644..0x15165A: A1 = 0x40B42E (dst), A0 = 0x14E19A (src),
     * D0.w = 0x10 - 1; jsr 0x16A19C copies D0.w+1 = 16 longs. */
    uint32_t dst = 0x40B42Eu;
    uint32_t src = 0x14E19Au;
    for (int i = 0; i < 16; i++) {
        wr32(m, dst, rd32(m, src));
        dst += 4u;
        src += 4u;
    }

    /* 0x151664..0x15167C: args (src, dst, count) for ROM 0x1660. */
    Pal_QueueUpload(m, 0x40B42Eu, 0x0B80u, 0x10u);

    /* 0x151660 restores D0/A0/A1 from the stack, so every register is
     * unchanged at the rts. */
    (void)r;
    return 0;
}
