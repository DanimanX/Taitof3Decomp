#include "fn_auto.h"
/* ROM 0x002274: UNVERIFIED (no capture): Scan runnable tasks, update sleep timers, and restore the selected task or idle context. */

int Fn_002274(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t status;
    uint32_t saved_sp;
    for (;;) {
        uint16_t id = rd16(m, globals - 0x1906u) + 1u;
        if (id == 0x20u) { /* 0x002280 -> 0x0022DE */
            if (!rd8(m, globals - 0x1452u) && rd8(m, globals - 0x1907u)) {
                wr8(m, globals - 0x1908u, 0xFFu);
                uint32_t idle_slot = globals - 0x194Cu;
                wr32(m, globals - 0x1900u, idle_slot);
                saved_sp = rd32(m, idle_slot);
                break; /* 0x00230C restores the idle context. */
            }
            wr8(m, globals - 0x1907u, 0xFFu);
            r->a[7] = globals - 0x1B4Cu;
            id = 0; /* 0x0022FA -> 0x002284 */
        }

        wr16(m, globals - 0x1906u, id);
        int32_t offset = (int16_t)(id << 2); /* 0x002288 ASL.W, then brief D1.w */
        uint32_t status_slot = globals - 0x18F8u + offset;
        wr32(m, globals - 0x18FCu, status_slot);
        status = rd32(m, status_slot);
        uint32_t active = rd32(m, globals - 0x1904u);
        if (!(active & (1u << (id & 31u))) || (status & 0x40000000u))
            continue;

        if (status & 0x80000000u) {
            uint16_t ticks = (uint16_t)(status - 1u);
            status = (status & 0xFFFF0000u) | ticks;
            wr32(m, status_slot, status); /* 0x0022AA */
            if (ticks)
                continue;
            status &= 0x3FFFFFFFu;
            wr16(m, status_slot, (uint16_t)(status >> 16)); /* 0x0022B6 */
        }

        uint32_t task_slot = globals - 0x1948u + offset;
        wr32(m, globals - 0x1900u, task_slot);
        saved_sp = rd32(m, task_slot);
        if (status & 0x20000000u) {
            wr32(m, status_slot, status & ~0x20000000u);
            wr32(m, saved_sp, rd32(m, globals - 0x18B8u + offset));
        }
        break; /* 0x0022D8 restores the selected task context. */
    }

    for (unsigned i = 0; i < 8; ++i)
        r->d[i] = rd32(m, saved_sp + 4u * i);
    for (unsigned i = 0; i < 7; ++i)
        r->a[i] = rd32(m, saved_sp + 32u + 4u * i);
    /* 0x0022DC / 0x002310 RTE consumes the scheduler's 8-byte format-0 frame.
     * Its SR and PC are outside f3regs_t. */
    r->a[7] = saved_sp + 60u + 8u;
    return 0;
}
