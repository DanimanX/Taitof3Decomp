/* ROM 0x122F88: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_122F88(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x122F88: Read long from A0+0xC into D0 */
    r->d[0] = rd32(m, r->a[0] + 0x0Cu);
    
    /* ROM 0x122F8C: Store low word of D0 to A0+0x22 */
    wr16(m, r->a[0] + 0x22u, (uint16_t)r->d[0]);
    
    /* ROM 0x122F90: Copy low word of D0 to D1, shift left by 6, store to A0+0x6 */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)r->d[0];
    r->d[1] = (r->d[1] & 0xFFFF0000u) | ((uint16_t)r->d[1] << 6);
    wr16(m, r->a[0] + 0x06u, (uint16_t)r->d[1]);
    
    /* ROM 0x122F98: Copy low word of D0 to D1, shift left by 4, add 0x10, store to A0+0x50 */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (uint16_t)r->d[0];
    r->d[1] = (r->d[1] & 0xFFFF0000u) | ((uint16_t)r->d[1] << 4);
    r->d[1] = (r->d[1] & 0xFFFF0000u) | ((uint16_t)r->d[1] + 0x10u);
    wr16(m, r->a[0] + 0x50u, (uint16_t)r->d[1]);
    
    /* ROM 0x122FA4: Store -0x14 to A0+0x52 */
    wr16(m, r->a[0] + 0x52u, 0xFFECu);
    
    /* ROM 0x122FAA: Clear word at A0+0x54 */
    wr16(m, r->a[0] + 0x54u, 0u);
    
    /* ROM 0x122FAE: Clear word at A0+0x56 */
    wr16(m, r->a[0] + 0x56u, 0u);
    
    /* ROM 0x122FB2: Clear word at A0+0x1E */
    wr16(m, r->a[0] + 0x1Eu, 0u);
    
    /* ROM 0x122FB6: Clear word at A0+0x1C */
    wr16(m, r->a[0] + 0x1Cu, 0u);
    
    /* ROM 0x122FBA: Return */
    return 0;
}
