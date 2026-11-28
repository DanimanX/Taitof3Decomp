/* ROM 0x0057DA: UNVERIFIED (no usable capture - every pair is
 * VBlank-interrupted and the test harness rejects them, and the injected calls
 * left garbage stack arguments): format an unsigned 8-digit decimal with
 * optional left padding. Stack args (A6 frame, A6 = entry SP-4): value.l at
 * SP+4, destination pointer.l at SP+8, pad byte.w at SP+0xC, field width.w at
 * SP+0xE. Byte-for-byte the same code as 0x000CB4; every register it uses is
 * saved by its movem, so it leaves the register file untouched. */
#include "fn_auto.h"

/* DIVU.W leaves the dividend unchanged if the quotient overflows. */
static uint32_t divu_word(uint32_t dividend, uint16_t divisor)
{
    uint32_t quotient = dividend / divisor;
    if (quotient > 0xFFFFu)
        return dividend;
    return ((dividend % divisor) << 16) | quotient;
}

int Fn_0057DA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t number = rd32(m, sp + 4u);                    /* 0x0057E2 (0x8,A6) */
    uint32_t dst = rd32(m, sp + 8u);                       /* 0x0057E6 (0xc,A6) */
    uint8_t pad = (uint8_t)rd16(m, sp + 0x0Cu);            /* 0x0057EA (0x10,A6) */
    uint16_t width = rd16(m, sp + 0x0Eu);                  /* 0x0057EE (0x12,A6) */

    number = divu_word(number, 10000);                     /* 0x0057F2 */

    uint32_t digit = (uint16_t)number;                     /* 0x0057F6..0x0057FC */
    uint16_t divisor = 1000;                               /* 0x005800 */
    uint16_t digits = 8;                                   /* 0x0057FE */
    /* 0x005806 / 0x005836: scan both four-digit groups, skipping leading zeroes. */
    for (;;) {
        digit = divu_word(digit, divisor);
        if ((uint16_t)digit != 0u || (int16_t)digits <= 1)          /* bhi / ble */
            break;
        digits--;
        if (digits == 4) {                                 /* 0x00581C */
            divisor = 1000;
            digit = number >> 16;
        } else {                                           /* 0x005826 */
            divisor = (uint16_t)divu_word(divisor, 10u);
            digit >>= 16;
        }
    }

    int16_t padding = (int16_t)(uint16_t)(width - digits);  /* 0x005836 sub.w */
    while (padding > 0) {                                  /* 0x005838 */
        wr8(m, dst++, pad);
        padding--;
    }

    for (;;) {                                             /* 0x005844 */
        wr8(m, dst++, (uint8_t)(digit | 0x30u));
        if ((int16_t)digits <= 1)                          /* 0x00584A ble */
            break;
        digits--;
        if (digits == 4) {                                 /* 0x005858 */
            divisor = 1000;
            digit = number >> 16;
        } else {                                           /* 0x005862 */
            divisor = (uint16_t)divu_word(divisor, 10u);
            digit >>= 16;
        }
        digit = divu_word(digit, divisor);                 /* 0x005870 */
    }
    wr8(m, dst, 0u);                                       /* 0x005874 */
    return 0;                                              /* 0x005878 movem ; unlk ; rts */
}
