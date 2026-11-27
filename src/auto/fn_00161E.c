#include "fn_auto.h"
/* ROM 0x00161E: dispatch the pending hardware palette transfers. */
#include "fn_call_frames.h"

/* 0x00164E / 0x001656: DBF changes only the counter's low word. */
static int dbf(uint32_t *reg)
{
    uint16_t count = (uint16_t)(*reg - 1u);
    *reg = set_word(*reg, count);
    return count != 0xFFFFu;
}

int Fn_00161E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t state = rd16(m, a5 - 0x5DEEu);
    /* 0x001624 is the extension-word base, with displacement +2 and D0.w*1. */
    uint32_t target = 0x001626u + (int16_t)state;
    if (target != 0x001626u && target != 0x001628u)
        return -1;

    r->d[0] = set_word(r->d[0], state);
    if (target == 0x001628u) {
        r->a[0] = 0x440000u;
        wr16(m, a5 - 0x5DEEu, 0);
        r->a[2] = a5 - 0x6750u;
        r->d[3] = set_word(r->d[3], rd16(m, a5 - 0x5DF0u));
        /* Both loops enter at DBF and run the original word count times. */
        while (dbf(&r->d[3])) {
            uint32_t entry = r->a[2];
            r->d[2] = set_word(r->d[2], rd16(m, entry));
            r->a[3] = rd32(m, entry + 2);
            r->d[1] = set_word(r->d[1], rd16(m, entry + 6));
            while (dbf(&r->d[2])) {
                uint32_t value = rd32(m, r->a[3]);
                r->a[3] += 4;
                /* 0x001648: the stock f3mem_t has no backing for palette RAM. */
                wr32(m, r->a[0] + (int16_t)r->d[1], value);
                r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] + 4u));
            }
            r->a[2] += 8;
        }
        wr16(m, a5 - 0x5DF0u, 0);
    }
    r->a[7] += 4;
    return 0;
}
