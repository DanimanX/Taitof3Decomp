#include "fn_auto.h"
/* ROM 0x004C78: scale the seven packed settings nibbles into timing words. */
#include "fn_call_frames.h"

int Fn_004C78(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->d[2]);
    r->a[0] = 0x406C54u;
    r->a[1] = 0x40888Cu;
    r->d[1] = 0x30u;

    /* 0x004C86 extracts {14:2}, the low two bits of the word at A0+0xA. */
    uint32_t scale = r->d[1] + ((rd8(m, r->a[0] + 0xBu) & 3u) << 4);
    /* 0x004C94..0x004CFE extract the high nibble first from each byte. */
    for (unsigned i = 0; i < 7; ++i) {
        uint8_t packed = rd8(m, r->a[0] + 0x14u + i / 2u);
        unsigned shift = 4u - 4u * (i % 2u);
        r->d[0] = (((packed >> shift) & 0xFu) + 1u) * scale;
        wr16(m, r->a[1] + 2u * i, (uint16_t)r->d[0]);
    }

    r->d[2] = rd32(m, r->a[7]);
    r->a[7] += 8u;
    return 0;
}
