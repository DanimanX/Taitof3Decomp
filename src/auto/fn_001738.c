#include "fn_auto.h"
/* ROM 0x001738: add the palette ramp deltas and pack the component high bytes. */
#include "fn_call_frames.h"

int Fn_001738(const f3mem_t *m, f3regs_t *r)
{
    r->d[4] = r->a[2];
    r->d[5] = r->a[3];
    r->d[6] = set_word(r->d[6], (uint16_t)(r->d[6] - 1u));

    /* 0x00173E..0x001766: each 12-byte ramp entry has three component
     * words followed by three delta words. Commit each sum before reading
     * its high byte for the 0x00RRGGBB output. */
    do {
        r->d[0] = 0;
        for (uint32_t offset = 0; offset < 6u; offset += 2u) {
            uint32_t component = r->a[2] + offset;
            uint16_t delta = rd16(m, component + 6u);
            r->d[1] = set_word(r->d[1], delta);
            wr16(m, component, (uint16_t)(rd16(m, component) + delta));
            r->d[0] = (r->d[0] << 8) | rd8(m, component);
        }
        wr32(m, r->a[3], r->d[0]);
        r->a[3] += 4u;
        r->a[2] += 12u;
        /* 0x00176A: DBF decrements D6.w before testing for underflow. */
        r->d[6] = set_word(r->d[6], (uint16_t)(r->d[6] - 1u));
    } while ((uint16_t)r->d[6] != 0xFFFFu);

    r->a[2] = r->d[4];
    r->a[3] = r->d[5];
    r->a[7] += 4u;
    return 0;
}
