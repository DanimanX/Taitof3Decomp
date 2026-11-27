#include "fn_auto.h"
/* ROM 0x000FA8: UNVERIFIED (weak capture): pace and play pending coin sounds. */
#include "fn_sound_frames.h"

int Fn_000FA8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t pending = a5 - 0x7F52u, delay = a5 - 0x7F51u;
    if (rd8(m, pending)) {
        if (rd8(m, delay)) {
            wr8(m, delay, rd8(m, delay) - 1u);  /* 0x000FE6 */
        } else {
            wr8(m, delay, 1);
            wr8(m, pending, rd8(m, pending) - 1u);
            if (rd8(m, a5 - 0x5DC7u)) {
                uint32_t inhibit = a5 - 0x1444u;
                r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, inhibit);
                wr8(m, inhibit, 0);
                call_hooked_sound(m, r, 0x37u, 0x000FD6u, 0x000FDCu);
                wr8(m, inhibit, (uint8_t)r->d[0]);
            }
        }
    }
    r->a[7] += 4;
    return 0;
}
