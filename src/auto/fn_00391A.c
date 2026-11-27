#include "fn_auto.h"
/* ROM 0x00391A: update two counters for consecutive matching byte samples. */

int Fn_00391A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t first_counter = a5 - 0x0C90u;
    uint32_t second_counter = a5 - 0x0C8Fu;

    /* 0x00391A–0x003938 */
    uint8_t sample = rd8(m, a5 - 0x5DDCu);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | sample;
    if (sample != rd8(m, a5 - 0x5DDAu)) {
        wr8(m, first_counter, 0);
    } else {
        uint8_t count = rd8(m, first_counter);
        if (count != 0xFFu)
            wr8(m, first_counter, (uint8_t)(count + 1u));
    }

    /* 0x00393C–0x00395A */
    sample = rd8(m, a5 - 0x5DD8u);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | sample;
    if (sample != rd8(m, a5 - 0x5DD6u)) {
        wr8(m, second_counter, 0);
    } else {
        uint8_t count = rd8(m, second_counter);
        if (count != 0xFFu)
            wr8(m, second_counter, (uint8_t)(count + 1u));
    }

    r->a[7] += 4u; /* 0x00395E: RTS */
    return 0;
}
