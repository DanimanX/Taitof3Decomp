#ifndef FN_PALETTE_FRAMES_H
#define FN_PALETTE_FRAMES_H

#include "fn_call_frames.h"

/* 0x001792 and 0x0017D6 use the same slot and RGB calculations as
 * Pal_QueueRamp. The frame here places the JSR after the slot writes. */
static inline uint32_t queue_palette_ramp(const f3mem_t *m, f3regs_t *r,
                                          uint32_t src, uint32_t dst,
                                          uint32_t start, uint32_t ticks,
                                          uint32_t count)
{
    uint32_t a5 = r->a[5];
    uint32_t slot = a5 - 0x6650u;
    for (unsigned i = 0; i < 16; ++i, slot += 6u) {
        if (rd16(m, slot) != 0u)
            continue;
        wr16(m, slot + 2u, (uint16_t)ticks);
        if ((int16_t)ticks <= 0)
            break;
        wr16(m, slot, (uint16_t)count);
        if ((uint16_t)count == 0u)
            break;
        wr16(m, slot + 4u, (uint16_t)start);
        push_long(m, r, 0x0017B8u);                  /* 0x0017B4 JSR */

        uint32_t record = a5 - 0x7F50u + 0x180u * i;
        for (uint16_t n = (uint16_t)count; n != 0u; --n, record += 12u) {
            uint32_t from = rd32(m, src);
            uint32_t to = rd32(m, dst);
            src += 4u;
            dst += 4u;
            for (unsigned channel = 0; channel < 3; ++channel) {
                unsigned shift = 16u - 8u * channel;
                int32_t initial = (int32_t)(((from >> shift) & 0xFFu) << 8);
                int32_t target = (int32_t)(((to >> shift) & 0xFFu) << 8);
                wr16(m, record + 2u * channel, (uint16_t)initial);
                /* Division extension $3803 selects a 32-bit dividend. */
                int32_t delta = (target - initial) / (int32_t)ticks;
                wr16(m, record + 6u + 2u * channel, (uint16_t)delta);
            }
        }
        r->a[7] += 4u;
        return slot;
    }
    return UINT32_MAX;                               /* 0x0017C8..0x0017CA */
}

/* 0x00183C, 0x001866, 0x001890, 0x0018BA and 0x0018E4 share the
 * LINK/MOVEM frame and return the result of 0x001792 in D0. */
static inline int call_palette_ramp(const f3mem_t *m, f3regs_t *r,
                                    uint32_t src, uint32_t dst,
                                    uint32_t wrapper_ret, uint32_t ramp_ret,
                                    int two_pointers)
{
    uint32_t sp = r->a[7];
    uint32_t start = r->d[1];
    uint32_t ticks = r->d[2];
    uint32_t count = r->d[3];
    push_long(m, r, count);
    push_long(m, r, ticks);
    push_long(m, r, start);
    if (two_pointers) {
        push_long(m, r, dst);
        push_long(m, r, src);
    } else {
        push_long(m, r, r->d[0]);
    }
    push_long(m, r, wrapper_ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);

    push_long(m, r, ramp_ret);
    push_long(m, r, r->a[3]);                         /* 0x001796 MOVEM */
    push_long(m, r, r->a[2]);
    push_long(m, r, dst);
    for (unsigned i = 8; i > 3;)
        push_long(m, r, r->d[--i]);
    push_long(m, r, count);
    push_long(m, r, ticks);
    push_long(m, r, start);

    r->d[0] = queue_palette_ramp(m, r, src, dst, start, ticks, count);
    r->a[7] = sp + 4u;                               /* wrapper LEA and RTS */
    return 0;
}

#endif
