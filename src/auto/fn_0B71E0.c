#include "fn_auto.h"

/* ROM 0x0B71E0: (recipe capture fnk_, work/zcap/recipes15.tsv): clear the 39-by-28 text region. */
#include "fn_call_frames.h"

int Fn_0B71E0(const f3mem_t *m, f3regs_t *r)
{
    /* 0x0B71E0: MOVEM.L A0/D2-D0,-(SP). */
    push_long(m, r, r->a[0]);
    for (unsigned i = 3; i > 0;)
        push_long(m, r, r->d[--i]);

    /* 0x0B71EE/0x0B71F4: DBF starts at 0x1B rows / 0x26 words. */
    uint32_t row = 0x61CE82u;
    for (unsigned y = 0; y < 28u; y++, row += 0x80u)
        for (unsigned x = 0; x < 39u; x++)
            wr16(m, row + 2u * x, 0u);

    /* 0x0B7204 restores all working registers, then 0x0B7208 pops the PC. */
    r->a[7] += 20u;
    return 0;
}
