/* ROM 0x0B7D52: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_0B7D52(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0B7D52: Read word at A5-0xC3C into D0 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)rd16(m, F3_A5 - 0xC3Cu);
    
    /* ROM 0x0B7D56: Compare D0 with 15 */
    /* ROM 0x0B7D5A: Branch if D0 > 15 (signed) to 0x0B7D68 */
    if ((int16_t)r->d[0] > 15) {
        /* ROM 0x0B7D68: Mask D0 with 0xF */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)(r->d[0] & 0xFu);
        /* ROM 0x0B7D6C: Write D1 to A1 + 0x200 + D0*2 */
        wr16(m, r->a[1] + 0x200u + (uint32_t)(int16_t)r->d[0] * 2u, (uint16_t)r->d[1]);
    } else {
        /* ROM 0x0B7D5E: Write D1 to A1 + 0x80 + D0*2 */
        wr16(m, r->a[1] + 0x80u + (uint32_t)(int16_t)r->d[0] * 2u, (uint16_t)r->d[1]);
    }
    
    /* ROM 0x0B7D72: RTS */
    return 0;
}
