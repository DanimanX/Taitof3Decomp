#include "fn_auto.h"
/* ROM 0x15CD38: blend and queue eighteen consecutive palette uploads. */
#include "fn_call_frames.h"
#include "../video/palette.h"

int Fn_15CD38(const f3mem_t *m, f3regs_t *r)
{
    static const uint16_t weights[] = {
        0xFF00, 0xEE00, 0xDD00, 0xCC00, 0xBB00, 0xAA00, 0x9900, 0x8800,
        0x7700, 0x6600, 0x5500, 0x4400, 0x3300, 0x2200, 0x1100, 0x0000,
        0x0000, 0x0000
    };

    /* 0x15CD38..0x15D1DB repeat the same call frame every 0x42 bytes. */
    for (unsigned i = 0; i < sizeof(weights) / sizeof(weights[0]); ++i) {
        uint32_t block = 0x15CD38u + i * 0x42u;
        uint32_t output = 0x40B72Eu + i * 0x40u;
        for (unsigned reg = 3; reg > 0;)
            push_long(m, r, r->a[--reg]);
        for (unsigned reg = 4; reg > 0;)
            push_long(m, r, r->d[--reg]);

        push_long(m, r, 0x023B40u);
        push_long(m, r, 0x15D5A4u);
        push_long(m, r, output);
        r->a[7] += 12u;
        r->a[0] = output;
        r->a[1] = 0x023B40u;
        r->a[2] = 0x15D5A4u;
        push_long(m, r, block + 0x26u);
        Pal_Blend64(m, output, r->a[1], r->a[2], weights[i]);
        r->a[7] += 4u;
        r->d[0] = set_word(r->d[0], (uint16_t)~weights[i]);
        r->d[1] = set_word(r->d[1], weights[i]);
        /* 0x16A270..0x16A280 leave the products for byte zero on the stack
         * through the following 0x001660 call, before the outer MOVEM. */
        r->d[3] = ((uint32_t)rd8(m, r->a[2]) << 8) * weights[i];
        uint32_t sum = ((uint32_t)rd8(m, r->a[1]) << 8) * (uint16_t)r->d[0]
                     + r->d[3];
        r->d[2] = ((sum % 0xFFFFu) << 16) | ((sum / 0xFFFFu) >> 8);
        r->d[7] = set_word(r->d[7], 0xFFFFu);

        FnAuto_QueuePalette(m, r, output, (uint16_t)(0x1C00u + i * 0x40u),
                           0x10u, block + 0x3Au);
        for (unsigned reg = 0; reg < 4; ++reg, r->a[7] += 4u)
            r->d[reg] = rd32(m, r->a[7]);
        for (unsigned reg = 0; reg < 3; ++reg, r->a[7] += 4u)
            r->a[reg] = rd32(m, r->a[7]);
    }
    r->a[7] += 4u; /* 0x15D1DC */
    return 0;
}
