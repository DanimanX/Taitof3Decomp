#include "fn_auto.h"

/* ROM 0x13D1E6: scale bytes 1 through 3 of 32 palette longs by 11/16. */
int Fn_13D1E6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t src = r->a[2];
    uint32_t dst = r->a[1];
    uint16_t d0 = 0x1Fu;
    uint32_t d1 = 0;

    do {
        uint32_t off = (uint32_t)d0 * 4u;
        uint32_t s = rd32(m, src + off);
        uint32_t d = rd32(m, dst + off);
        uint8_t v = (uint8_t)((s >> 16) & 0xFFu);
        d = (d & 0xFF00FFFFu) | ((uint32_t)((v * 11u) >> 4) << 16);
        v = (uint8_t)((s >> 8) & 0xFFu);
        d = (d & 0xFFFF00FFu) | ((uint32_t)((v * 11u) >> 4) << 8);
        v = (uint8_t)s;
        d = (d & 0xFFFFFF00u) | ((v * 11u) >> 4);
        wr32(m, dst + off, d);
        d1 = (v * 11u) >> 4;
    } while (--d0 != 0xFFFFu);

    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0xFFFFu;
    r->d[1] = d1;
    return 0;
}
