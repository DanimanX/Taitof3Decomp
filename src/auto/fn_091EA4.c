#include "fn_auto.h"
/* ROM 0x091EA4: UNVERIFIED (weak capture): select a direction in D1.W from active-low input bits. */
#include "fn_call_frames.h"

int Fn_091EA4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t input = r->a[5] - 0x5DDBu;
    if (!(rd8(m, input) & 8u))                                   /* 0x091EAA */
        r->d[1] = set_word(r->d[1], 1);
    if (!(rd8(m, input) & 4u))                                   /* 0x091EB8 */
        r->d[1] = set_word(r->d[1], 0);
    r->a[7] += 4u;                                               /* 0x091EC0 RTS */
    return 0;
}
