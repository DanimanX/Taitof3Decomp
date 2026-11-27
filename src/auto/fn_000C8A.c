#include "fn_auto.h"
/* ROM 0x000C8A: queue one serial byte, reporting whether the index advanced. */
#include "fn_call_frames.h"

int Fn_000C8A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    r->a[0] = a5 - 0x8000u;
    uint16_t index = rd16(m, a5 - 0x7F80u);
    r->d[0] = set_word(r->d[0], index);
    wr8(m, r->a[0] + (int16_t)index, rd8(m, r->a[7] + 4));
    index++;
    if (index == 0x80)
        index = 0;
    if (index == rd16(m, a5 - 0x7F7Eu)) {
        r->d[0] = 0;
    } else {
        wr16(m, a5 - 0x7F80u, index);
        r->d[0] = 1;
    }
    r->a[7] += 4;
    return 0;
}
