/* ROM 0x0C6836: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_0C6836(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0C6836: Load pointer from A5-0xCA0 into A1 */
    r->a[1] = rd32(m, r->a[5] - 0xCA0u);
    /* ROM 0x0C683A: Store D6 at A1 */
    wr32(m, r->a[1], r->d[6]);
    /* ROM 0x0C683C: D0.w &= D4.w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (r->d[0] & r->d[4] & 0xFFFFu);
    /* ROM 0x0C683E: D1.w &= D4.w */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | (r->d[1] & r->d[4] & 0xFFFFu);
    /* ROM 0x0C6840: Store D0.w at A1+4 */
    wr16(m, r->a[1] + 4u, r->d[0] & 0xFFFFu);
    /* ROM 0x0C6844: Store D1.w at A1+6 */
    wr16(m, r->a[1] + 6u, r->d[1] & 0xFFFFu);
    /* ROM 0x0C6848: Store D5 at A1+8 */
    wr32(m, r->a[1] + 8u, r->d[5]);
    /* ROM 0x0C684C: Clear word at A1+12 */
    wr16(m, r->a[1] + 12u, 0u);
    /* ROM 0x0C6850: A1 += 16 */
    r->a[1] += 16u;
    /* ROM 0x0C6854: Store A1 back to A5-0xCA0 */
    wr32(m, r->a[5] - 0xCA0u, r->a[1]);
    /* ROM 0x0C6858: Return */
    return 0;
}
