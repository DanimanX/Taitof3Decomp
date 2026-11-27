#include "fn_auto.h"
/* ROM 0x004BD2: sum a settings block and replace its final word with the checksum. */
#include "fn_call_frames.h"

int Fn_004BD2(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    r->a[1] = rd32(m, r->a[7] + 0xCu);

    /* 0x004BE0..0x004BF4: signed byte count divided by two, toward zero.
     * The $2A00 extension selects D2.L scaled by two. */
    int32_t last = (int32_t)rd32(m, r->a[7] + 0x10u) / 2 - 1;
    uint16_t sum = 0;
    uint32_t word = r->a[1];
    for (int32_t i = 0; i < last; ++i, word += 2u)
        sum = (uint16_t)(sum + rd16(m, word));

    /* 0x004BF6..0x004C08: A0 retains the full signed sum including
     * the old final word. D0 returns its low word zero-extended. */
    r->a[0] = (uint32_t)((int32_t)(int16_t)sum + (int16_t)rd16(m, word));
    r->d[0] = (uint16_t)r->a[0];
    r->d[1] = (uint32_t)(-(int32_t)(int16_t)sum);
    wr16(m, word, (uint16_t)r->d[1]);
    r->d[2] = rd32(m, r->a[7]);
    r->d[3] = rd32(m, r->a[7] + 4u);
    r->a[7] += 12u;
    return 0;
}
