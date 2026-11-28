/* ROM 0x0B7B28: (palette capture fnp_): copy 0x20 longs from (A1) into the
 * 0x40-byte-strided cell array at 0x440500.
 *
 * The destination cursor A2 starts at 0x440500 and is advanced by 4 BEFORE each
 * slot test, so the first candidate is 0x440504; a slot is used when
 * (A2 & 0x3F) is neither 0 nor above 0x20 - that is, when the low six bits name
 * one of the 0x20 long slots 1..0x20 of a 0x40-byte group. The source pointer
 * advances by 4 per copied long; the destination pointer does not advance past
 * the store, it is re-advanced by the next iteration.
 *
 * movem.l {A2 A1 D1 D0},-(SP) / (SP)+ restores every register, so the routine
 * leaves the whole register file as it found it. */
#include "fn_auto.h"

int Fn_0B7B28(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a2 = 0x00440500u;                       /* 0x0B7B2C lea (0x440500),A2 */
    uint32_t a1 = r->a[1];
    uint16_t d0 = 0x001Fu;                           /* 0x0B7B32 move.w #0x1F,D0 */
    do {
        uint16_t d1;
        for (;;) {
            a2 += 4u;                                /* 0x0B7B36 addq.l #4,A2 */
            d1 = (uint16_t)(a2 & 0x003Fu);           /* 0x0B7B38..0x0B7B3A */
            if (d1 != 0u && (int16_t)d1 <= 0x20)     /* 0x0B7B3E beq / 0x0B7B44 bgt */
                break;
        }
        wr32(m, a2, rd32(m, a1));                    /* 0x0B7B46 move.l (A1)+,(A2) */
        a1 += 4u;
        d0--;                                        /* 0x0B7B48 dbf D0 */
    } while ((int16_t)d0 >= 0);
    return 0;                                        /* 0x0B7B4C..0x0B7B50 restore, rts */
}
