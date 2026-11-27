/* ROM 0x1630A2: clear playfield 0, fill 0x1D0 bytes of line RAM at
 * 0x62B400 + (A5-0x5D56) with 0xB00A through 0x176198, set +0x44 = 0x80 and
 * hand the object to 0x1630D2. */
#include "fn_auto.h"
#include "../video/sprite.h"

/* ROM 0x176198: fill 0xE8 words (0x1D0 bytes) at base + (A5-0x5D56) with val.
 * The ROM writes them backwards from base+off+0x1D0 with movem.l bursts; the
 * whole range gets the same value, so the order is irrelevant. */
static void fill_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0u; off < 0x01D0u; off += 2u)
        wr16(m, a0 + off, val);                             /* ROM 0x1761C4..0x1761FC */
}

int Fn_1630A2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Gfx_ClearPf0(m);                                        /* ROM 0x1630A2: jsr 0x000058DA */
    /* ROM 0x1630A8..0x1630C0: movem.l {A0},-(SP) ; move.w #-0x4FF6,-(SP) ;
     * pea 0x62B400 ; jsr 0x176198 ; lea (6,SP),SP ; movem.l (SP)+,{A0} */
    fill_176198(m, 0x0062B400u, 0xB00Au);
    wr16(m, obj + 0x44u, 0x0080u);                          /* ROM 0x1630C4: move.w #0x80,(0x44,A0) */
    wr32(m, obj, 0x001630D2u);                              /* ROM 0x1630CA: move.l #0x1630d2,(A0) */
    return 0;                                               /* ROM 0x1630D0 rts */
}
