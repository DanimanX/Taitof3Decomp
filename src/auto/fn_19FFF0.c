#include "fn_auto.h"
/* ROM 0x19FFF0: adjust a bounded menu value with an accelerating repeat counter. */
#include "fn_call_frames.h"

int Fn_19FFF0(const f3mem_t *m, f3regs_t *r)
{
    /* 0x19FFF0: predecrement MOVEM pushes D5, D4, then D3. */
    push_long(m, r, r->d[5]);
    push_long(m, r, r->d[4]);
    push_long(m, r, r->d[3]);

    uint32_t sp = r->a[7];
    uint16_t buttons = rd16(m, sp + 0x10u);
    uint16_t value = rd16(m, sp + 0x12u);
    r->a[0] = 0x40CBD0u;
    if ((buttons & 0xCu) == 0xCu) {
        wr16(m, r->a[0], 0); /* 0x1A0062 */
    } else {
        int16_t held = (int16_t)rd16(m, r->a[0]);
        if (held == 0 || held > 0x14) {
            unsigned repeats = held >= 0x78 ? 10u : 1u;
            for (unsigned i = 0; i < repeats; ++i) {
                uint16_t step = rd16(m, sp + 0x18u);
                if ((buttons & 8u) == 0)
                    value = (uint16_t)(value + step);
                else
                    value = (uint16_t)(value - step);
                /* 0x1A003E-0x1A004E: signed upper clamp, then lower clamp. */
                uint16_t upper = rd16(m, sp + 0x16u);
                if ((int16_t)value > (int16_t)upper)
                    value = upper;
                uint16_t lower = rd16(m, sp + 0x14u);
                if ((int16_t)value < (int16_t)lower)
                    value = lower;
            }
        }
        if (held < 0x78)
            wr16(m, r->a[0], (uint16_t)(held + 1));
    }

    r->d[0] = set_word(r->d[0], value);
    r->d[1] = set_word(r->d[1], value);
    r->d[3] = rd32(m, sp);
    r->d[4] = rd32(m, sp + 4u);
    r->d[5] = rd32(m, sp + 8u);
    r->a[7] += 16u;
    return 0;
}
