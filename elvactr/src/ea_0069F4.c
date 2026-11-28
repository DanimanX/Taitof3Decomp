/* ROM 0x0069F4: copy bytes from a0 to a1, doubling the count byte d7 for each non-zero source byte */
#include "ea_auto.h"

int Ea_0069F4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a6 = sp - 4u;
    uint32_t sp_movem = a6 - 12u;

    /* 0x0069F4 link.w a6, #$0 */
    wr32(m, a6, r->a[6]);

    /* 0x0069F8 movem.l d7/a0-a1, -(a7) */
    wr32(m, sp_movem, r->d[7]);
    wr32(m, sp_movem + 4u, r->a[0]);
    wr32(m, sp_movem + 8u, r->a[1]);

    /* 0x0069FC movea.l $8(a6), a0 */
    uint32_t a0 = rd32(m, a6 + 8u);
    /* 0x006A00 movea.l $c(a6), a1 */
    uint32_t a1 = rd32(m, a6 + 12u);
    /* 0x006A04 move.w $10(a6), d7 */
    uint16_t d7 = rd16(m, a6 + 16u);

    /* 0x006A08 add.w d7, d7 */
    d7 <<= 1;

    /* 0x006A0A loop start */
    for (;;) {
        /* 0x006A0A move.b (a0)+, d0 */
        uint32_t d0 = rd8(m, a0);
        a0++;
        /* 0x006A0C beq.b $6a14 */
        if (d0 == 0) break;
        /* 0x006A0E move.b d7, (a1)+ */
        wr8(m, a1, (uint8_t)d7);
        a1++;
        /* 0x006A10 move.b d0, (a1)+ */
        wr8(m, a1, (uint8_t)d0);
        a1++;
    }

    /* 0x006A14 move.l a1, d0 */
    r->d[0] = a1;

    /* 0x006A16 movem.l (a7)+, d7/a0-a1 */
    r->d[7] = rd32(m, sp_movem);
    r->a[0] = rd32(m, sp_movem + 4u);
    r->a[1] = rd32(m, sp_movem + 8u);

    /* 0x006A1A unlk a6 */
    r->a[6] = rd32(m, a6);
    a6 += 4u;
    r->a[7] = rd32(m, a6);
    a6 += 4u;

    /* 0x006A1C rts */
    r->a[7] += 4u;

    return 0;
}
