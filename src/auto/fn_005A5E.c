/* ROM 0x005A5E: clear the text layer attribute words at 0x61C000, 29 rows of
 * 0x80 bytes apart:
 *
 *   005A5E  movem.l {D0 D1 A0},-(SP)
 *   005A62  A0 = 0x61C000 ; D0 = 0x1C ; D1 = 0 (move.w #0) ; lsl.w #8 ; add.w
 *   005A72  D1b = 1                          -> D1w = 0x0001
 *   005A76  loop: word (A0) = D1w ; A0 += 0x80 ; dbf D0, loop
 *   005A80  movem.l (SP)+,{D0 D1 A0} ; rts
 *
 * Every register is restored; only the 29 gfx words change. */
#include "fn_auto.h"

int Fn_005A5E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], a0 = r->a[0];
    uint32_t dst = 0x0061C000u;
    uint16_t count = 0x1Cu;
    uint16_t value = 0x0001u;

    r->a[7] = sp - 12u;                            /* 0x005A5E movem.l {D0 D1 A0} */
    wr32(m, r->a[7] + 0u, d0);
    wr32(m, r->a[7] + 4u, d1);
    wr32(m, r->a[7] + 8u, a0);

    do {
        wr16(m, dst, value);                       /* 0x005A76 move.w D1w,(A0) */
        dst += 0x80u;                              /* 0x005A78 lea 0x80(A0),A0 */
        count = (uint16_t)(count - 1u);            /* 0x005A7C dbf D0 */
    } while (count != 0xFFFFu);

    r->d[0] = rd32(m, r->a[7] + 0u);                /* 0x005A80 movem.l (SP)+ */
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->a[0] = rd32(m, r->a[7] + 8u);
    r->a[7] = sp + 4u;                              /* the rts pops the caller's PC */
    return 0;
}
