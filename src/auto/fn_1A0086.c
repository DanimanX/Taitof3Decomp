/* ROM 0x1A0086: draw one volume bar into text RAM.
 *
 *   jsr 0x1A0086 with, from the caller's SP+4: dst.l, value.l, slots.l,
 *   colour.w  (the push order is colour, slots, value, dst).
 *
 * The routine clears `slots` text cells at dst, then paints value/8 full cells
 * (character = colour<<9 | 0x18) and, when value is not a multiple of 8, one
 * partial cell (colour<<9 | 0x10 | value%8) in the next slot. D2/D3/D4 are
 * saved and restored by the movem pair, D1 ends as 9 (the `moveq #9,D1` before
 * the shift), D0 as the last cell word and A0/A1 as the two cursors (A0 after
 * the full cells, A1 past the cleared run) - the sound-test screens leave them
 * that way to the next segment, so they are modelled exactly.
 *
 * The 12 saved bytes go below SP where no capture can see them, but the same
 * layout is written back by the movem pair at 0x1A00E0.
 *
 * UNVERIFIED on its own (no usable capture): fn_1A0086 exists, but every pair
 * was sampled one frame after its entry, so test_fn skips all 30 as
 * VBlank-interrupted. The routine is verified through the 0x1A00E6 group,
 * whose seg_1A0168 capture compares the A0/A1 cursors it leaves. */
#include "../kernel/seg.h"     /* f3regs_t, seg_setw (fn_auto.h comes along) */

int Fn_1A0086(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t sp = r->a[7];                /* return PC at (SP) */
    uint32_t a0 = rd32(m, sp + 4u);              /* 0x1A008A movea.l (0x10,SP),A0 */
    const uint32_t value = rd32(m, sp + 8u);     /* 0x1A008E move.l (0x14,SP),D0 */
    const uint32_t slots = rd32(m, sp + 0xCu);   /* 0x1A00AC (0x18,SP) */
    const uint16_t colour = rd16(m, sp + 0x10u); /* 0x1A00B6 (0x1C,SP) */

    wr32(m, sp - 4u, r->d[4]);                   /* 0x1A0086 predecrement MOVEM */
    wr32(m, sp - 8u, r->d[3]);
    wr32(m, sp - 12u, r->d[2]);

    /* 0x1A0092 bpl / 0x1A0094 addq.l #7 / 0x1A0096 asr.l #3 */
    r->d[0] = value;
    if ((int32_t)r->d[0] < 0)
        r->d[0] += 7u;
    r->d[0] = (uint32_t)((int32_t)r->d[0] >> 3);
    r->d[4] = r->d[0];                           /* 0x1A0098 move.l D0,D4 */
    r->d[3] = 7u;                                /* 0x1A009A moveq #7,D3 */
    r->d[3] &= value;                            /* 0x1A009C and.l (0x14,SP),D3 */

    uint32_t a1 = a0;                            /* 0x1A00A0 movea.l A0,A1 */
    for (r->d[2] = 0u; (int32_t)r->d[2] < (int32_t)slots; r->d[2] += 1u) {
        wr16(m, a1, 0u);                         /* 0x1A00A6 clr.w (A1) */
        a1 += 2u;
    }
    for (r->d[2] = 0u; (int32_t)r->d[2] < (int32_t)r->d[4]; r->d[2] += 1u) {
        seg_setw(&r->d[0], colour);              /* 0x1A00B6 move.w (0x1C,SP),D0w */
        r->d[1] = 9u;                            /* 0x1A00BC moveq #9,D1 */
        r->d[0] <<= (r->d[1] & 63u);             /* 0x1A00BE lsl.l D1,D0 */
        r->d[0] = (r->d[0] & 0xFFFF0000u) |       /* 0x1A00C0 addi.w #0x18,D0w */
                  (uint16_t)((r->d[0] + 0x18u) & 0xFFFFu);
        wr16(m, a0, (uint16_t)r->d[0]);           /* 0x1A00C4 move.w D0w,(A0) */
        a0 += 2u;
    }
    seg_setw(&r->d[0], colour);                  /* 0x1A00CE move.w (0x1C,SP),D0w */
    r->d[1] = 9u;
    r->d[0] <<= (r->d[1] & 63u);                 /* 0x1A00D6 lsl.l D1,D0 */
    r->d[0] += r->d[3];                          /* 0x1A00D8 add.l D3,D0 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) |           /* 0x1A00DA addi.w #0x10,D0w */
              (uint16_t)((r->d[0] + 0x10u) & 0xFFFFu);
    wr16(m, a0, (uint16_t)r->d[0]);               /* 0x1A00DE move.w D0w,(A0) */

    r->d[2] = rd32(m, sp - 12u);                  /* 0x1A00E0 movem.l (SP)+,d2-d4 */
    r->d[3] = rd32(m, sp - 8u);
    r->d[4] = rd32(m, sp - 4u);
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[7] = sp + 4u;                            /* 0x1A00E4 rts */
    return 0;
}
