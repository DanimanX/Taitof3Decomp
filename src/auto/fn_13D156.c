#include "fn_auto.h"
/* ROM 0x13D156: queue 60-frame fades to darkened stage palettes. */
#include "../objects/obj_common.h"

/* ROM 0x13D1E6: scale the RGB bytes of 32 colors in descending order. */
static void darken_palette(const f3mem_t *m, f3regs_t *r)
{
    for (uint32_t color = 32; color-- > 0;) {
        for (uint32_t channel = 1; channel <= 3; ++channel) {
            uint32_t offset = color * 4 + channel;
            r->d[1] = rd8(m, r->a[2] + offset) * 11u / 16u;
            wr8(m, r->a[1] + offset, (uint8_t)r->d[1]);
        }
    }
    r->d[0] |= 0xFFFFu;
    r->a[7] += 4;
}

/* ROM 0x00183C: preserve the wrapper's stack frames around Pal_QueueRamp.
 * The caller restores D0 at 0x13D198, including after a failed claim. */
static void queue_stage_ramp(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7], frame = sp - 4;
    wr32(m, frame, r->a[6]);                         /* 0x00183C */
    wr32(m, frame - 4, r->a[1]);
    wr32(m, frame - 8, r->a[0]);
    wr32(m, frame - 12, r->d[2]);
    wr32(m, frame - 16, r->d[1]);
    uint32_t src = rd32(m, sp + 4), dst = rd32(m, sp + 8);
    uint32_t index = rd32(m, sp + 12);
    uint32_t duration = rd32(m, sp + 16), count = rd32(m, sp + 20);
    wr32(m, frame - 20, 0x00185C);
    uint32_t saved = frame - 20;
    wr32(m, saved -= 4, r->a[3]);
    wr32(m, saved -= 4, r->a[2]);
    wr32(m, saved -= 4, dst);
    for (unsigned i = 8; i-- > 3;)
        wr32(m, saved -= 4, r->d[i]);
    wr32(m, saved - 4, count);
    wr32(m, saved - 8, duration);
    wr32(m, saved - 12, index);
    uint32_t slot = Pal_QueueRamp(m, src, dst, index, duration, count);
    if (slot)
        wr32(m, saved - 16, 0x0017B8);
    r->d[0] = slot ? slot : UINT32_MAX;
    r->a[7] = sp + 4;
}

int Fn_13D156(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    for (unsigned i = 3; i-- > 0;)
        wr32(m, r->a[7] -= 4, r->a[i]);
    wr32(m, r->a[7] -= 4, r->d[1]);
    wr32(m, r->a[7] -= 4, r->d[0]);

    /* Full PC-index extension 0x0520, base 0x13D160 plus 0x00C6. */
    int32_t stage = (int16_t)rd16(m, r->a[5] + 0x32DC);
    r->a[0] = rd32(m, 0x13D226u + stage * 4);
    r->a[1] = 0x41B58E;
    while ((int16_t)rd16(m, r->a[0]) >= 0) {          /* 0x13D16C */
        r->a[2] = rd32(m, r->a[0]);
        r->a[0] += 4;
        wr32(m, r->a[7] -= 4, 0x13D178);
        darken_palette(m, r);
        r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, r->a[0]);
        r->a[0] += 2;
        wr32(m, r->a[7] -= 4, 32);
        wr32(m, r->a[7] -= 4, 60);
        wr32(m, r->a[7] -= 4, r->d[0]);
        wr32(m, r->a[7] -= 4, r->a[1]);
        wr32(m, r->a[7] -= 4, r->a[2]);
        wr32(m, r->a[7] -= 4, 0x13D192);
        queue_stage_ramp(m, r);
        r->a[7] += 20;
    }
    r->d[0] = rd32(m, sp - 20);
    r->d[1] = rd32(m, sp - 16);
    for (unsigned i = 0; i < 3; ++i)
        r->a[i] = rd32(m, sp - 12 + i * 4);
    r->a[7] = sp + 4;
    return 0;
}
