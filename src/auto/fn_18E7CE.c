/* ROM 0x18E7CE: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_18E7CE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x18E7CE: Decrement word at A0+6 by 1 */
    uint16_t val = rd16(m, r->a[0] + 6);
    val--;
    wr16(m, r->a[0] + 6, val);
    
    /* ROM 0x18E7D2: Branch to 0x18E7C6 if result > 0 (signed) */
    if ((int16_t)val > 0) {
        /* ROM 0x18E7C6: Store 0x18E7CE at A0, then RTS */
        wr32(m, r->a[0], 0x18E7CEu);
        return 0;
    }
    
    /* ROM 0x18E7D4: A4 = *(A0+8) */
    r->a[4] = rd32(m, r->a[0] + 8);
    
    /* ROM 0x18E7D8: *(A4+0x3A) = 0x18E954 */
    wr32(m, r->a[4] + 0x3Au, 0x18E954u);
    
    /* ROM 0x18E7E0: *(A4+0x3E) = 0 */
    wr32(m, r->a[4] + 0x3Eu, 0u);
    
    /* ROM 0x18E7E4: Branch to 0x18E186 */
    /* ROM 0x18E186: *(A0+6) = 2 */
    wr16(m, r->a[0] + 6, 2u);
    /* ROM 0x18E18C: beq.b 0x18E19C (never taken since immediate is non-zero) */
    /* ROM 0x18E18E: *(A0) = 0x18E196 */
    wr32(m, r->a[0], 0x18E196u);
    /* ROM 0x18E194: RTS */
    return 0;
}
