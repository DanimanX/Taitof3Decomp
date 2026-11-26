/* ROM 0x1330D0: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_1330D0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1330D0: Decrement timer at A5+0x30D6, decrement counter at A0+6 */
    wr32(m, F3_A5 + 0x30d6u, rd32(m, F3_A5 + 0x30d6u) - 0x4000u);
    wr16(m, r->a[0] + 6u, (uint16_t)(rd16(m, r->a[0] + 6u) - 1u));

    /* ROM 0x1330DC: If counter > 0, jump to 0x1330C8 */
    if ((int16_t)rd16(m, r->a[0] + 6u) > 0)
        goto LAB_001330c8;

    /* ROM 0x1330DE: Set counter to 0x1E, fall through */
    wr16(m, r->a[0] + 6u, 0x001Eu);
    /* ROM 0x1330E4: beq.b 0x1330F4 (not taken, falls through) */
    goto LAB_001330e6;

LAB_001330c8:
    /* ROM 0x1330C8: Store return address 0x1330D0 to A0, return */
    wr32(m, r->a[0], 0x1330D0u);
    return 0;

LAB_001330e6:
    /* ROM 0x1330E6: Store return address 0x1330EE to A0, return */
    wr32(m, r->a[0], 0x1330EEu);
    return 0;
}
