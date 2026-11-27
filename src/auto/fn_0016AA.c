#include "fn_auto.h"
/* ROM 0x0016AA: advance palette ramps and drain the timed palette upload script. */
#include "fn_call_frames.h"
#include "../video/palette.h"

/* 0x001660: both call sites push three longs. LINK and MOVEM save the
 * caller's A6 and D0-D2/A0/A2 below the return address. */
static void queue_upload(const f3mem_t *m, f3regs_t *r, uint32_t source,
                         uint32_t destination, uint32_t count, uint32_t return_pc)
{
    push_long(m, r, count);
    push_long(m, r, destination);
    push_long(m, r, source);
    push_long(m, r, return_pc);
    push_long(m, r, r->a[6]);
    uint32_t frame = r->a[7];
    push_long(m, r, r->a[2]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);

    /* 0x00167C..0x00169A: the normal queue helper uses the game's A5
     * base and nonnegative indices. The ROM shifts negative indices in a
     * word and then sign-extends that word at ADDA.W. */
    uint32_t queue_count = r->a[5] - 0x5DF0u;
    int16_t index = (int16_t)rd16(m, queue_count);
    if (r->a[5] == F3_A5 && index >= 0) {
        Pal_QueueUpload(m, source, (uint16_t)destination, (uint16_t)count);
    } else if (index < 0x20) {
        int16_t offset = (int16_t)((uint16_t)index << 3);
        uint32_t slot = r->a[5] - 0x6750u + offset;
        wr16(m, slot, (uint16_t)count);
        wr32(m, slot + 2u, source);
        wr16(m, slot + 6u, (uint16_t)destination);
        wr16(m, queue_count + 2u, 2u);
        wr16(m, queue_count, (uint16_t)(rd16(m, queue_count) + 1u));
    }

    /* 0x00169E..0x0016A8: MOVEM, UNLK, RTS, then discard the three args. */
    r->d[0] = rd32(m, frame - 20u);
    r->d[1] = rd32(m, frame - 16u);
    r->d[2] = rd32(m, frame - 12u);
    r->a[0] = rd32(m, frame - 8u);
    r->a[2] = rd32(m, frame - 4u);
    r->a[6] = rd32(m, frame);
    r->a[7] = frame + 20u;
}

int Fn_0016AA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    r->a[1] = globals - 0x6650u;
    r->a[2] = globals - 0x7F50u;
    r->a[3] = globals - 0x65F0u;
    r->d[7] = 15u;

    /* 0x0016B8..0x0016E8: each nonempty slot advances once, even when
     * the upload queue is full. A zero timer after SUBQ clears the slot. */
    do {
        uint32_t slot = r->a[1];
        r->d[6] = set_word(r->d[6], rd16(m, slot));
        if ((uint16_t)r->d[6] != 0) {
            push_long(m, r, 0x0016C0u);
            Fn_001738(m, r);
            r->d[0] = set_word(r->d[0], rd16(m, slot + 4u));
            r->d[1] = set_word(r->d[1], rd16(m, slot));
            queue_upload(m, r, r->a[3], r->d[0], r->d[1], 0x0016D0u);

            uint16_t timer = (uint16_t)(rd16(m, slot + 2u) - 1u);
            wr16(m, slot + 2u, timer);
            if (timer == 0)
                wr16(m, slot, 0);
        }
        r->a[1] += 6u;
        r->a[2] += 0x180u;
        r->a[3] += 0x80u;
        r->d[7] = set_word(r->d[7], (uint16_t)(r->d[7] - 1u));
    } while ((uint16_t)r->d[7] != 0xFFFFu);

    /* 0x0016EC..0x001736: the script supplies count, source offset and
     * destination words. Keep the cursor on the first unqueued entry. */
    uint32_t remaining = globals - 0x5DE8u;
    if ((int16_t)rd16(m, remaining) > 0) {
        uint32_t cursor = globals - 0x5DECu;
        r->a[1] = rd32(m, cursor);
        r->d[1] = rd16(m, globals - 0x5DE6u) == 0 ? 0x16900u : 0x1FB40u;
        r->d[0] = set_word(r->d[0], 0x20u);
        while ((int16_t)r->d[0] > (int16_t)rd16(m, globals - 0x5DF0u)) {
            r->d[2] = set_word(r->d[2], rd16(m, r->a[1]));
            r->a[1] += 2u;
            r->d[3] = rd16(m, r->a[1]) + r->d[1];
            r->a[1] += 2u;
            r->d[4] = set_word(r->d[4], rd16(m, r->a[1]));
            r->a[1] += 2u;
            queue_upload(m, r, r->d[3], r->d[4], r->d[2], 0x001728u);

            wr32(m, cursor, r->a[1]);
            uint16_t count = rd16(m, remaining);
            wr16(m, remaining, (uint16_t)(count - 1u));
            if ((int16_t)count <= 1)
                break;
        }
    }
    r->a[7] += 4u;
    return 0;
}
