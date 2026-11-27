/* ROM 0x164654: fill two 0x1D0-byte line-RAM bands at 0x62B000 (0xB00A) and
 * 0x62B400 (0xB008) through 0x176198, then set +0x44 = 0x40 and hand the
 * object to 0x164692. */
#include "fn_auto.h"

/* ROM 0x176198: fill 0xE8 words (0x1D0 bytes) at base + (A5-0x5D56) with val.
 * The ROM writes them backwards from base+off+0x1D0 with movem.l bursts; the
 * whole range receives the same value, so the order is irrelevant. */
static void fill_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0u; off < 0x01D0u; off += 2u)
        wr16(m, a0 + off, val);                             /* ROM 0x1761C4..0x1761FC */
}

int Fn_164654(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* ROM 0x164654..0x16467C: movem.l {A0},-(SP) ; move.w #-0x4FF6,-(SP) ;
     * pea 0x62B000 ; jsr 0x176198 ; lea (6,SP),SP ; move.w #-0x4FF8,-(SP) ;
     * pea 0x62B400 ; jsr 0x176198 ; lea (6,SP),SP ; movem.l (SP)+,{A0} */
    fill_176198(m, 0x0062B000u, 0xB00Au);
    fill_176198(m, 0x0062B400u, 0xB008u);
    wr16(m, obj + 0x44u, 0x0040u);                          /* ROM 0x164684: move.w #0x40,(0x44,A0) */
    wr32(m, obj, 0x00164692u);                              /* ROM 0x16468A: move.l #0x164692,(A0) */
    return 0;                                               /* ROM 0x164690 rts */
}
