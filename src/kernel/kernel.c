/* Kernel: task scheduler and the TRAP system calls (see kernel.h). */
#include "kernel.h"

/* ROM 0x002274: scheduler loop. Scan runnable tasks, update sleep timers, and
 * restore the selected task or idle context, up to (not including) the RTE at
 * 0x0022DC (task) or 0x002310 (idle). Verified 40/40 (tests/test_fn.c,
 * work/ram/seg_002274: 36 task, 4 idle restores). */
uint32_t Kern_Schedule(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t status;
    uint32_t saved_sp;
    uint32_t rte_pc = 0x000022DCu;
    for (;;) {
        uint16_t id = rd16(m, globals - 0x1906u) + 1u;
        if (id == 0x20u) { /* 0x002280 -> 0x0022DE */
            if (!rd8(m, globals - 0x1452u) && rd8(m, globals - 0x1907u)) {
                wr8(m, globals - 0x1908u, 0xFFu);
                uint32_t idle_slot = globals - 0x194Cu;
                wr32(m, globals - 0x1900u, idle_slot);
                saved_sp = rd32(m, idle_slot);
                rte_pc = 0x00002310u;
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
    /* Stops at the RTE (0x0022DC / 0x002310); its SR/PC frame stays on the
     * restored stack, so A7 points at it. */
    r->a[7] = saved_sp + 60u;
    return rte_pc;
}

/* The TRAP handlers below run from their entry to the RTE that ends them (or
 * to the bra into Kern_Schedule) with r = the registers at the trap: A7 points
 * at the 68020 exception frame (SR.w, PC.l, format/vector word), so the
 * caller's arguments start at SP+8. Every memory store the ROM makes is
 * written, including the movem save areas: the ones a handler pops again land
 * below SP, the ones the scheduler restores from (TRAP #4/#5/#6/#7) and the
 * task stack TRAP #1 builds are read later. */

/* 0x1F34 / 0x1F7C / 0x1FB0 / 0x1FEC: movem.l d0-d7/a0-a6,-(a7) ;
 * movea.l -$1900(a5),a6 ; move.l a7,(a6). The scheduler restores the context
 * from that block, so it is written out. Returns the new A7 (the saved SP). */
static uint32_t push_context(const f3mem_t *m, const f3regs_t *r)
{
    uint32_t ctx = r->a[7] - 60u;
    for (unsigned i = 0; i < 8; ++i)
        wr32(m, ctx + 4u * i, r->d[i]);
    for (unsigned i = 0; i < 7; ++i)
        wr32(m, ctx + 32u + 4u * i, r->a[i]);
    wr32(m, rd32(m, r->a[5] - 0x1900u), ctx);
    return ctx;
}

/* ROM 0x001E76 TRAP #1: create a task in slot (SP+12) (0 = first free slot,
 * a value >= 0x10 searches downward from 15). Claims the slot in the active
 * mask, clears its -$1878 word, sets its status to (current id << 16), and
 * builds the task context (d0-d7/a0-a6, RTE frame PC = (SP+8), return to
 * TRAP #3 0x1E48, one stack argument (SP+16)) on the ROM stack table at
 * 0x2368. D0 = the slot used (the full argument when it named one), 0 when no
 * slot is free. Verified 20/20 (tests/test_fn.c, work/ram/fn_001E76). */
uint32_t Kern_001E76(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t d0 = rd32(m, sp + 12u);          /* 0x1E84 */
    uint32_t mask = rd32(m, globals - 0x1904u);
    uint32_t id;

    if (d0 == 0) {                            /* 0x1E8A: first free slot */
        id = 0;
        while (mask & (1u << (id & 31u))) {
            if (++id >= 0x10u) {              /* 0x1E90 */
                r->d[0] = 0;                  /* 0x1E96 */
                return 0x00001F18u;
            }
        }
        d0 = id;
    } else if ((uint16_t)d0 < 0x10u) {
        id = (uint16_t)d0;                    /* 0x1E9E */
    } else {                                  /* 0x1EA0: downward from 15 */
        int found = -1;
        for (uint32_t k = 0x10u; k-- > 0u;)
            if (!(mask & (1u << (k & 31u)))) { found = (int)k; break; }
        if (found < 0) {
            r->d[0] = 0;                      /* 0x1EAA */
            return 0x00001F18u;
        }
        id = (uint32_t)found;
        d0 = id;
    }

    if (mask & (1u << (id & 31u))) {          /* 0x1EB0 bset: already active */
        r->d[0] = 0;                          /* 0x1EB4 */
        return 0x00001F18u;
    }
    mask |= 1u << (id & 31u);
    wr32(m, globals - 0x1904u, mask);         /* 0x1EB8 */

    uint16_t index = (uint16_t)(2u * id);     /* 0x1EBE add.w d7,d7 */
    if (index < 0x20u)                        /* 0x1EC0 cmpi.w #$20,d7 */
        wr16(m, globals - 0x1878u + (int16_t)index, 0xFFFFu);   /* 0x1EC6 */
    index = (uint16_t)(index + index);        /* 0x1ECC: 4*id */
    wr32(m, globals - 0x18F8u + (int16_t)index,  /* 0x1ED6 */
         (uint32_t)rd16(m, globals - 0x1906u) << 16);

    /* 0x1EDA: the per-slot stack top from the ROM table at 0x2368. */
    uint32_t p = rd32(m, 0x2368u + (int16_t)index);
    p -= 4u; wr32(m, p, rd32(m, sp + 16u));            /* 0x1EE2 */
    p -= 4u; wr32(m, p, 0x00001E48u);                  /* 0x1EE6 task exit */
    p -= 2u; wr16(m, p, 0x0000u);                      /* 0x1EEC frame format */
    p -= 4u; wr32(m, p, rd32(m, sp + 8u));             /* 0x1EEE task PC */
    p -= 2u; wr16(m, p, 0x2000u);                      /* 0x1EF2 SR */
    p -= 20u;                                          /* 0x1EF6 movem a2-a6 */
    for (unsigned i = 0; i < 5; ++i)
        wr32(m, p + 4u * i, r->a[2 + i]);
    p -= 4u; wr32(m, p, r->a[1]);                      /* 0x1EFA */
    p -= 4u; wr32(m, p, r->a[0]);                      /* 0x1EFE */
    p -= 4u; wr32(m, p, r->d[7]);                      /* 0x1F02 */
    p -= 4u; wr32(m, p, r->d[6]);                      /* 0x1F06 */
    p -= 24u;                                          /* 0x1F08 movem d0-d5 */
    wr32(m, p, d0);         /* D0 was reused as the chosen slot (0x1E8E/0x1EAE) */
    for (unsigned i = 1; i < 6; ++i)
        wr32(m, p + 4u * i, r->d[i]);
    wr32(m, globals - 0x1948u + (int16_t)index, p);    /* 0x1F10 */

    r->d[0] = d0;                             /* 0x1F14 pops d6-d7/a0-a1 */
    return 0x00001F18u;
}

/* ROM 0x001E0E TRAP #2: make the tasks selected by (SP+8) other than the
 * current one runnable again: active mask &= ~(((SP+8) & ~bit(id)) & ~1). No
 * register changes (D0/D1 are pushed and popped). Verified 2/2
 * (work/ram/fn_001E0E). */
uint32_t Kern_001E0E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t id = rd16(m, globals - 0x1906u);
    uint32_t d0 = rd32(m, r->a[7] + 8u);      /* 0x1E16 */
    d0 &= ~(1u << (id & 31u));                /* 0x1E1E bclr.b d1,d0 */
    d0 &= 0xFFFFFFFEu;                        /* 0x1E20 */
    wr32(m, globals - 0x1904u,                /* 0x1E28 */
         rd32(m, globals - 0x1904u) & ~d0);
    return 0x00001E30u;                       /* 0x1E2C restores D0/D1 */
}

/* ROM 0x001E48 TRAP #3: task exit. Clear the current task's bit in the active
 * mask and let the scheduler pick the next context. D0 = the new mask,
 * D1.w = the task id. When the current id is 0, 0x1E54 branches to itself.
 * Verified 3/3 (work/ram/fn_001E48). */
uint32_t Kern_001E48(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t d0 = rd32(m, globals - 0x1904u);
    uint32_t d1 = (r->d[1] & 0xFFFF0000u) | rd16(m, globals - 0x1906u);
    r->d[0] = d0;
    r->d[1] = d1;
    if ((uint16_t)d1 == 0)                    /* 0x1E54 beq $1e54 */
        return 0x00001E54u;
    d0 &= ~(1u << (d1 & 31u));                /* 0x1E56 bclr.b d1,d0 */
    r->d[0] = d0;
    wr32(m, globals - 0x1904u, d0);
    return Kern_Schedule(m, r);               /* 0x1E5C bra 0x002274 */
}

/* ROM 0x001F30 TRAP #4: sleep the current task for (SP+10) ticks. Save the
 * register set as the task context, set its status to
 * ((id | 0xA000) << 16) | ticks (bit 31 run, bit 29 wake-with-stack-fix) and
 * clear its -$18B8 slot, then reschedule. Verified 20/20
 * (work/ram/fn_001F30). */
uint32_t Kern_001F30(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t ctx = push_context(m, r);        /* 0x1F34 */
    uint16_t id = rd16(m, globals - 0x1906u);
    uint32_t d0 = ((uint32_t)(id | 0xA000u) << 16) | rd16(m, sp + 10u);
    wr32(m, rd32(m, globals - 0x18FCu), d0);  /* 0x1F52 */
    wr32(m, globals - 0x18B8u + 4u * id, 0);  /* 0x1F5A clr.l (a4,d1.w) */
    r->d[0] = d0;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)(4u * id);
    r->a[4] = globals - 0x18B8u;
    r->a[6] = rd32(m, globals - 0x1900u);
    r->a[7] = ctx;
    return Kern_Schedule(m, r);               /* 0x1F5E bra 0x002274 */
}

/* ROM 0x001F78 TRAP #5: yield. Save the register set as the task context and
 * clear bits 31/30 of the task status (+ stopping the sleep timer), then
 * reschedule. D0.w = status >> 16 & 0x3FFF. Verified 20/20
 * (work/ram/fn_001F78). */
uint32_t Kern_001F78(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t ctx = push_context(m, r);        /* 0x1F7C */
    uint32_t slot = rd32(m, globals - 0x18FCu);
    uint16_t status = (uint16_t)(rd16(m, slot) & 0x3FFFu);
    wr16(m, slot, status);                    /* 0x1F90 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | status;
    r->a[4] = slot;
    r->a[6] = rd32(m, globals - 0x1900u);
    r->a[7] = ctx;
    return Kern_Schedule(m, r);               /* 0x1F92 bra 0x002274 */
}

/* ROM 0x001FAC TRAP #7: sleep the current task for (SP+10) ticks without the
 * wake-with-stack-fix bit: status = ((id << 16) | ticks) | 0x40000000. Saves
 * the register set as the task context and reschedules. UNVERIFIED (no
 * capture). */
uint32_t Kern_001FAC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t ctx = push_context(m, r);        /* 0x1FB0 */
    uint32_t d0 = ((uint32_t)rd16(m, globals - 0x1906u) << 16) |
                  rd16(m, sp + 10u);          /* 0x1FBA / 0x1FC0 */
    d0 |= 0x40000000u;                        /* 0x1FC4 bset #$1e,d0 */
    wr32(m, rd32(m, globals - 0x18FCu), d0);  /* 0x1FCC */
    r->d[0] = d0;
    r->a[4] = rd32(m, globals - 0x18FCu);
    r->a[6] = rd32(m, globals - 0x1900u);
    r->a[7] = ctx;
    return Kern_Schedule(m, r);               /* 0x1FCE bra 0x002274 */
}

/* ROM 0x001FE8 TRAP #6: wait for event (SP+8) & 0xF of the current task.
 * Saves the register set as the task context. When the task's -$1878 word
 * already has that bit, status = ((id | 0x6000) << 16) | bit and it
 * reschedules; otherwise the word gets the bit, status byte +1 = the bit
 * number, and D0 = the long at -$1878+0x20+16*id+4*bit (written over the saved
 * D0, so it is the RTE's D0) before the routine returns. UNVERIFIED (no
 * capture). */
uint32_t Kern_001FE8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t ctx = push_context(m, r);        /* 0x1FEC */
    uint16_t id = rd16(m, globals - 0x1906u);
    if (id > 0x0Fu)                           /* 0x2002 */
        return 0x00002044u;                   /* 0x2040 pops d0-d7/a0-a6 */
    uint32_t slot = rd32(m, globals - 0x18FCu);
    uint32_t event = rd32(m, sp + 8u) & 0x0Fu;            /* 0x2012 */
    uint32_t wslot = globals - 0x1878u + 2u * id;
    uint32_t word = rd16(m, wslot);
    uint32_t bit = 1u << (event & 31u);
    if (word & bit) {                         /* 0x201A bset: already set */
        uint32_t d0 = ((uint32_t)(id | 0x6000u) << 16) | bit;
        wr32(m, slot, d0);                    /* 0x2028 */
        r->d[0] = d0;
        r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)(2u * id); /* 0x200C add.w d1,d1 */
        r->d[2] = (r->d[2] & 0xFFFF0000u) | (uint16_t)(word | bit); /* 0x200E/0x201A */
        r->d[3] = (rd32(m, sp + 8u) & 0xFFFF0000u) | event;   /* 0x2012/0x2016 */
        r->a[3] = slot;
        r->a[4] = globals - 0x1878u;
        r->a[6] = rd32(m, globals - 0x1900u);
        r->a[7] = ctx;
        return Kern_Schedule(m, r);           /* 0x202A bra 0x002274 */
    }
    wr16(m, wslot, (uint16_t)(word | bit));   /* 0x202E */
    wr8(m, slot + 1u, (uint8_t)event);        /* 0x2032 move.b d3,$1(a3) */
    uint32_t d0 = rd32(m, globals - 0x1878u + 0x20u + 16u * id + 4u * event);
    wr32(m, ctx, d0);                         /* 0x203C move.l ...,(a7) */
    r->d[0] = d0;
    return 0x00002044u;
}

/* ROM 0x00205C TRAP #8: wake every task in the mask (SP+8) except the caller.
 * For each set bit: status = (status & 0x20000000 when bit 30 is clear else
 * status) | (1 << id) | 0x40000000, and the -$1878 word gets 1 << id. No
 * register changes (all five saved registers are popped). UNVERIFIED (no
 * capture). */
uint32_t Kern_00205C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint16_t id = rd16(m, globals - 0x1906u);
    if (id > 0x0Fu)                           /* 0x206C */
        return 0x000020B0u;
    uint32_t d0 = rd32(m, r->a[7] + 8u);      /* 0x2070 */
    d0 &= ~(1u << (id & 31u));                /* 0x2074 bclr.b d2,d0 */
    uint32_t d1 = (1u << (id & 31u)) | 0x40000000u;       /* 0x2078/0x207A */
    uint32_t words = globals - 0x1878u;
    uint32_t status = globals - 0x18F8u;
    for (;;) {
        int carry = (int)(d0 & 1u);           /* 0x2086 lsr.l #1 */
        d0 >>= 1u;
        if (carry) {                          /* 0x208E */
            uint32_t st = rd32(m, status);
            if (!(st & 0x40000000u)) st &= 0x20000000u;
            wr32(m, status, st | d1);         /* 0x209E */
            wr16(m, words, (uint16_t)(rd16(m, words) | (d1 & 0xFFFFu)));
            words += 2u;                      /* 0x20A6 */
            status += 4u;
        } else if (d0 == 0) {
            break;                            /* 0x208A -> 0x20AC */
        } else {
            words += 2u;
            status += 4u;
        }
    }
    return 0x000020B0u;                       /* 0x20AC pops d0-d2/a3-a4 */
}

/* ROM 0x0020C8 TRAP #9: start/stop task (SP+8) (>= 16 is ignored).
 *  (SP+8) <= 15 and the target status has bit 30 and bit id: clear bit id;
 *  when its low word then reads 0, the new status is
 *  (((status >> 16) & 0x2000) | id) << 16 and (SP+12) becomes the target's
 *  saved stack; -$1878 word gets bit id.
 *  Otherwise: -$1878 word loses bit id and (SP+12) goes to
 *  -$18B8 + 16*(SP+8) + 4*id. No register changes. UNVERIFIED (no capture). */
uint32_t Kern_0020C8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint32_t sp = r->a[7];
    uint16_t id = rd16(m, globals - 0x1906u);
    if (id > 0x0Fu)                           /* 0x20D4 */
        return 0x0000213Eu;
    uint32_t arg = rd32(m, sp + 8u);          /* 0x20DC */
    if ((uint16_t)arg > 0x0Fu)                /* 0x20E0 */
        return 0x0000213Eu;
    uint32_t other = (uint16_t)arg;
    uint32_t slots = globals - 0x18B8u;
    uint32_t status_slot = slots - 0x40u + 4u * other;    /* -$40(a4,d0.w) */
    uint32_t word_slot = slots + 0x40u + 2u * other;      /* $40(a4,d3.w) */
    uint32_t bit = 1u << (id & 31u);
    uint32_t status = rd32(m, status_slot);
    if ((status & 0x40000000u) && (status & bit)) {       /* 0x20F6/0x20FE */
        uint32_t rest = status & ~bit;
        if ((uint16_t)rest != 0) {
            wr32(m, status_slot, rest);       /* 0x2114 */
        } else {
            wr32(m, slots + 4u * other, rd32(m, sp + 12u));          /* 0x210E */
            wr32(m, status_slot,              /* 0x2114 after 0x2104-0x210C */
                 (((status >> 16) & 0x2000u) | id) << 16);
        }
        wr16(m, word_slot, (uint16_t)(rd16(m, word_slot) | bit));
    } else {
        wr16(m, word_slot, (uint16_t)(rd16(m, word_slot) & ~bit));
        wr32(m, slots + 0x60u + 16u * other + 4u * id,
             rd32(m, sp + 12u));              /* 0x2134 */
    }
    return 0x0000213Eu;                       /* 0x213A pops d0-d3/a4 */
}

/* ROM 0x002156 TRAP #10: for every task in the mask (SP+8) except the caller:
 * clear bit id in its status; when that leaves the low word 0, drop the task
 * to status (id << 16) and, if bit 29 was set, clear its -$18B8 slot. No
 * register changes. UNVERIFIED (no capture). */
uint32_t Kern_002156(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint16_t id = rd16(m, globals - 0x1906u);
    if (id > 0x0Fu)                           /* 0x2162 */
        return 0x000021A6u;
    uint32_t d0 = rd32(m, r->a[7] + 8u);      /* 0x216A */
    d0 &= ~(1u << (id & 31u));                /* 0x216E bclr.b d2,d0 */
    uint32_t status = globals - 0x18F8u;
    uint32_t bit = 1u << (id & 31u);
    for (;;) {
        int carry = (int)(d0 & 1u);           /* 0x2174 lsr.l #1 */
        d0 >>= 1u;
        if (carry) {                          /* 0x217C */
            uint32_t st = rd32(m, status);
            if ((st & 0x40000000u) && (st & bit)) {
                st &= ~bit;
                if ((uint16_t)st != 0) {
                    wr32(m, status, st);      /* 0x219C */
                } else {
                    if (st & 0x20000000u)     /* 0x218C btst #$1d */
                        wr32(m, status + 0x40u, 0);          /* 0x2192 */
                    wr32(m, status, (uint32_t)id << 16);
                }
            }
        } else if (d0 == 0) {
            break;                            /* 0x2178 -> 0x21A2 */
        }
        status += 4u;                         /* 0x219E */
    }
    return 0x000021A6u;                       /* 0x21A2 pops d0-d2/a4 */
}

/* ROM 0x0021BE TRAP #11: hand task slot (SP+8) & 0x1F to the caller: its
 * status becomes (id << 16) | (old status bit 29 ? 0x2000 : 0) and its saved
 * stack -$18B8 + 4*slot becomes (SP+12). No register changes. UNVERIFIED (no
 * capture). */
uint32_t Kern_0021BE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint16_t id = rd16(m, globals - 0x1906u);
    uint32_t slot = rd32(m, r->a[7] + 8u) & 0x1Fu;        /* 0x21C6-0x21CE */
    uint32_t status_slot = globals - 0x18F8u + 4u * slot;
    uint32_t status = rd32(m, status_slot);   /* 0x21D4 */
    /* 0x21E4 move.b d2,d1: only d1's low byte takes the id. */
    wr32(m, status_slot,                      /* 0x21E8 */
         ((uint32_t)(id & 0xFFu) << 16) | (status & 0x20000000u ? 0x2000u : 0u));
    wr32(m, globals - 0x18B8u + 4u * slot, rd32(m, r->a[7] + 12u));  /* 0x21EC */
    return 0x000021F6u;                       /* 0x21F2 pops d0-d2/a4 */
}

/* ROM 0x00220E TRAP #12: -$1878 word of the current task |= (SP+8).w. No
 * register changes. UNVERIFIED (no capture). */
uint32_t Kern_00220E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t globals = r->a[5];
    uint16_t index = (uint16_t)(2u * rd16(m, globals - 0x1906u)); /* 0x2222 */
    uint32_t wslot = globals - 0x1878u + (int16_t)index;
    wr16(m, wslot, (uint16_t)(rd16(m, wslot) | (uint16_t)rd32(m, r->a[7] + 8u)));
    return 0x0000222Cu;                       /* 0x2228 pops d2-d3/a4 */
}
