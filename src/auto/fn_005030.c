#include "fn_auto.h"
/* ROM 0x005030: install the default score table, clear the trailing entries and update its checksum. */
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

int Fn_005030(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[2]);
    r->a[2] = 0x406C54u;
    /* 0x00503A: five 12-byte default score records. */
    for (uint32_t i = 0; i < 5u; ++i) {
        uint32_t offset = i * 12u;
        r->a[0] = r->a[2] + 0x20u + offset;
        r->a[1] = 0x1A1BF8u + offset;
        for (unsigned word = 0; word < 3u; ++word) {
            wr32(m, r->a[0], rd32(m, r->a[1]));
            r->a[0] += 4u;
            r->a[1] += 4u;
        }
    }
    /* 0x005060: D1.L is scaled by two (extension word 0x1A5C). */
    for (uint32_t i = 0; i < 0x11u; ++i)
        wr16(m, r->a[2] + 0x5Cu + i * 2u, 0xFFFFu);
    r->d[0] = r->d[1] = 0x11u;

    push_long(m, r, 0x60u);
    push_long(m, r, r->a[2] + 0x20u);
    push_long(m, r, 0x00507Au);
    checksum_004BD2(m, r);
    r->a[7] += 8u;
    r->a[2] = rd32(m, r->a[7]);
    r->a[7] += 8u; /* 0x00507C: restore A2, then RTS. */
    return 0;
}
