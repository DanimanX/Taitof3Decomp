#include "fn_auto.h"
/* ROM 0x00144A: terminate the current sprite list and switch sprite RAM banks. */
#include "fn_call_frames.h"

int Fn_00144A(const f3mem_t *m, f3regs_t *r)
{
    /* 0x00144A: MOVEM.L to -(SP) stores in reverse register order. */
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);

    uint32_t cursor = r->a[5] - 0x0CA0u;
    uint32_t control = r->a[5] - 0x0CA4u;
    /* 0x001452: memory bit 16 is the high bit of the cursor's third byte. */
    uint32_t bank = rd8(m, cursor + 2u) >> 7;
    r->a[0] = r->a[5] - 0x0C9Cu + bank * 4u;
    uint32_t previous_end = rd32(m, r->a[0]);
    wr16(m, previous_end + 0xCu, 0);

    uint32_t list_end = rd32(m, cursor);
    wr16(m, list_end, 0);
    wr16(m, list_end + 4u, 0x0FF0u);
    wr16(m, list_end + 6u, 0x0FF0u);
    wr16(m, list_end + 8u, 0);
    /* 0x001478: ROR.W #5 moves the bank bit from bit 0 to bit 11. */
    wr16(m, list_end + 0xCu, (uint16_t)(0x83FFu | (bank << 11)));
    wr32(m, r->a[0], list_end);

    /* 0x001484: reread the cursor and replace its low 15 bits with 0x3FF0. */
    uint32_t bank_end = (rd32(m, cursor) & 0xFFFF8000u) | 0x3FF0u;
    wr16(m, bank_end, 0);
    wr16(m, bank_end + 4u, 0x0FF0u);
    wr16(m, bank_end + 6u, 0x0FF0u);
    wr16(m, bank_end + 8u, 0);
    wr16(m, bank_end + 0xCu, (uint16_t)(0x8000u | ((uint16_t)bank_end >> 4)));

    /* 0x0014B0: copy the bank toggle's low bit into control, then use
     * the complemented toggle's high bit to select the next bank. */
    uint32_t toggle = r->a[5] - 0x0CA2u;
    uint16_t old_toggle = rd16(m, toggle);
    wr16(m, toggle, (uint16_t)~rd16(m, toggle));
    wr8(m, control + 1u,
        (uint8_t)((rd8(m, control + 1u) & 0xFEu) | (old_toggle & 1u)));
    wr32(m, cursor, 0x600310u | ((uint16_t)~old_toggle & 0x8000u));

    /* 0x0014D0: BSR.W pushes 0x0014D4, which the callee's RTS pops. */
    push_long(m, r, 0x0014D4u);
    Fn_001262(m, r);

    /* 0x0014D4: restore D0/D1/A0/A1, then pop the caller's return address. */
    r->d[0] = rd32(m, r->a[7]);
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->a[0] = rd32(m, r->a[7] + 8u);
    r->a[1] = rd32(m, r->a[7] + 12u);
    r->a[7] += 20u;
    return 0;
}
