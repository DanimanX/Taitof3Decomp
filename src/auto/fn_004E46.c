#include "fn_auto.h"
/* ROM 0x004E46: install the EEPROM identification bytes and update the configuration checksum. */
#include "fn_call_frames.h"

/* ROM 0x004BD2: sum all words except the last, return the old sum including
 * the last word in D0.w, and replace the last word with the negated sum. */
static void checksum_004BD2(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    r->a[1] = rd32(m, r->a[7] + 0xCu);
    uint16_t sum = 0;
    uint32_t index = 0;
    for (;;) {
        /* 0x004BE6-0x004BF4: signed byte count divided by two, toward zero. */
        int32_t last = (int32_t)rd32(m, r->a[7] + 0x10u) / 2 - 1;
        if ((int32_t)index >= last)
            break;
        sum = (uint16_t)(sum + rd16(m, r->a[1] + index * 2u));
        ++index;
    }
    uint32_t last_word = r->a[1] + index * 2u;
    r->a[0] = (uint32_t)((int32_t)(int16_t)sum + (int32_t)(int16_t)rd16(m, last_word));
    r->d[0] = (uint16_t)r->a[0];
    r->d[1] = (uint32_t)(-(int32_t)(int16_t)sum);
    wr16(m, last_word, (uint16_t)r->d[1]); /* 0x004C08 */
    r->d[2] = rd32(m, r->a[7]);
    r->d[3] = rd32(m, r->a[7] + 4u);
    r->a[7] += 12u;
}

int Fn_004E46(const f3mem_t *m, f3regs_t *r)
{
    r->a[0] = 0x406C54u;
    r->a[1] = 0x1A1BD8u;
    for (unsigned i = 0; i < 5u; ++i)
        wr8(m, r->a[0] + i, rd8(m, r->a[1] + i));
    r->d[0] = set_word(r->d[0], rd16(m, 0x1FFFFEu));
    wr8(m, r->a[0] + 5u, (uint8_t)r->d[0]);

    /* 0x004E76: PEA size, push the table pointer, then JSR. */
    push_long(m, r, 0x20u);
    push_long(m, r, r->a[0]);
    push_long(m, r, 0x004E80u);
    checksum_004BD2(m, r);
    r->a[7] += 12u; /* discard arguments and return */
    return 0;
}
