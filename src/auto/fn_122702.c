/* ROM 0x122702: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void Fn_1226fa(const f3mem_t *m, f3regs_t *r) {
    (void)r;
    /* ROM 0x1226FA: Store 0x122702 to (A0) and return */
    wr32(m, r->a[0], 0x122702u);
}

int Fn_122702(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x122702: Decrement long at A5+0x30D2 by 0x10000 */
    wr32(m, F3_A5 + 0x30d2u, rd32(m, F3_A5 + 0x30d2u) - 0x10000u);

    /* ROM 0x12270A: Decrement word at A0+0x22 by 1 */
    uint16_t val = (uint16_t)(rd16(m, r->a[0] + 0x22u) - 1u);
    wr16(m, r->a[0] + 0x22u, val);

    /* ROM 0x12270E: Branch to 0x1226FA if result > 0 (signed) */
    if ((int16_t)val > 0) {
        Fn_1226fa(m, r);
        return 0;
    }

    /* ROM 0x122710: Move long from A0+0x54 to A5+0x30EE */
    wr32(m, F3_A5 + 0x30eeu, rd32(m, r->a[0] + 0x54u));

    /* ROM 0x122716: Move byte from A0+0x58 to A5+0x30F2 */
    wr8(m, F3_A5 + 0x30f2u, rd8(m, r->a[0] + 0x58u));

    /* ROM 0x12271C: Move long from A5+0x3264 to A1 */
    r->a[1] = rd32(m, F3_A5 + 0x3264u);

    /* ROM 0x122720: Move byte from A0+0x59 to A1+0x58 */
    wr8(m, r->a[1] + 0x58u, rd8(m, r->a[0] + 0x59u));

    /* ROM 0x122726: Move long from A5+0x3268 to A1 */
    r->a[1] = rd32(m, F3_A5 + 0x3268u);

    /* ROM 0x12272A: Move byte from A0+0x5A to A1+0x58 */
    wr8(m, r->a[1] + 0x58u, rd8(m, r->a[0] + 0x5au));

    /* ROM 0x122730: Move long from A0+0x50 to A0 */
    wr32(m, r->a[0], rd32(m, r->a[0] + 0x50u));

    /* ROM 0x122734: Return */
    return 0;
}
