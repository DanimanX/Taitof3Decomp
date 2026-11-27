#include "fn_auto.h"
/* ROM 0x000C30: format decimal digits and queue each pair as a packed byte. */
#include "fn_call_frames.h"

int Fn_000C30(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    r->a[6] = r->a[7];
    push_long(m, r, r->d[2]);
    r->d[0] = set_word(r->d[0], (uint16_t)(rd16(m, r->a[6] + 12) * 2u));
    push_word(m, r, (uint16_t)r->d[0]);
    push_word(m, r, 0x30u);
    push_long(m, r, r->a[5] - 0x7F7Cu);
    push_long(m, r, rd32(m, r->a[6] + 8));
    push_long(m, r, 0x000C50);
    Fn_000CB4(m, r);
    r->a[7] += 12;
    r->a[0] = r->a[5] - 0x7F7Cu;
    r->d[2] = set_word(r->d[2], (uint16_t)(rd16(m, r->a[6] + 12) - 1u));

    /* 0x000C5E: DBF is reached after the first pair, even for a zero count. */
    do {
        uint8_t high = (uint8_t)(rd8(m, r->a[0]++) - 0x30u);
        uint8_t low = (uint8_t)(rd8(m, r->a[0]++) - 0x30u);
        uint8_t packed = (uint8_t)((high << 4) | low);
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | packed;
        r->d[1] = (r->d[1] & 0xFFFFFF00u) | low;
        push_long(m, r, r->a[0]);
        r->a[7] -= 2;
        wr8(m, r->a[7], packed);
        push_long(m, r, 0x000C78);
        Fn_000C8A(m, r);
        r->a[7] += 2;
        r->a[0] = rd32(m, r->a[7]);
        r->a[7] += 4;
        r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] - 1u));
    } while ((uint16_t)r->d[2] != 0xFFFFu);

    r->d[2] = rd32(m, r->a[7]);
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8;
    return 0;
}
