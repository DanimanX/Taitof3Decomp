#include "fn_auto.h"
/* ROM 0x000CB4: format an unsigned number with optional left padding. */
#include "fn_call_frames.h"

/* DIVU.W leaves the dividend unchanged if the quotient overflows. */
static uint32_t divu_word(uint32_t dividend, uint16_t divisor)
{
    uint32_t quotient = dividend / divisor;
    if (quotient > 0xFFFFu)
        return dividend;
    return ((dividend % divisor) << 16) | quotient;
}

int Fn_000CB4(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    r->a[6] = r->a[7];
    /* 0x000CB8: predecrement MOVEM writes highest register first. */
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    for (unsigned reg = 5; reg > 0; reg--)
        push_long(m, r, r->d[reg]);

    uint32_t number = rd32(m, r->a[6] + 8);
    uint32_t dst = rd32(m, r->a[6] + 12);
    uint8_t pad = (uint8_t)rd16(m, r->a[6] + 16);
    uint16_t width = rd16(m, r->a[6] + 18);
    number = divu_word(number, 10000);

    uint32_t digit = (uint16_t)number;
    uint16_t divisor = 1000;
    uint16_t digits = 8;
    /* 0x000CE0 / 0x000D4A: scan both four-digit groups, skipping leading zeroes. */
    while (1) {
        digit = divu_word(digit, divisor);
        if ((uint16_t)digit || digits == 1)
            break;
        if (--digits == 4) {
            divisor = 1000;
            digit = number >> 16;
        } else {
            divisor /= 10;
            digit >>= 16;
        }
    }

    /* 0x000D10 uses a wrapping word subtraction and a signed pad check. */
    int16_t padding = (int16_t)(width - digits);
    for (; padding > 0; padding--)
        wr8(m, dst++, pad);

    while (1) {
        wr8(m, dst++, (uint8_t)(digit | 0x30u));
        if (digits == 1)
            break;
        if (--digits == 4) {
            divisor = 1000;
            digit = number >> 16;
        } else {
            divisor /= 10;
            digit >>= 16;
        }
        digit = divu_word(digit, divisor);
    }
    wr8(m, dst, 0);

    for (unsigned reg = 1; reg <= 5; reg++) {
        r->d[reg] = rd32(m, r->a[7]);
        r->a[7] += 4;
    }
    r->d[7] = rd32(m, r->a[7]);
    r->a[0] = rd32(m, r->a[7] + 4);
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8;
    return 0;
}
