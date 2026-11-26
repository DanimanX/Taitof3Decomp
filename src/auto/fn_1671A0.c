/* ROM 0x1671A0: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static void thunk_167198(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x167198: Store 0x1671A0 to (A0) */
    wr32(m, r->a[0], 0x1671A0u);
}

int Fn_1671A0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1671A0: Add 0x400 to long at A5+0x422C */
    uint32_t addr1 = F3_A5 + 0x422Cu;
    wr32(m, addr1, rd32(m, addr1) + 0x400u);

    /* ROM 0x1671A8: Add 0x100 to long at A5+0x4234 */
    uint32_t addr2 = F3_A5 + 0x4234u;
    wr32(m, addr2, rd32(m, addr2) + 0x100u);

    /* ROM 0x1671B0: Subtract 1 from word at A0+0x44 */
    uint32_t addr3 = r->a[0] + 0x44u;
    uint16_t val = (uint16_t)rd16(m, addr3);
    val--;
    wr16(m, addr3, val);

    /* ROM 0x1671B4: Branch to thunk if result >= 0 (signed) */
    if ((int16_t)val >= 0) {
        thunk_167198(m, r);
        return 0;
    }

    /* ROM 0x1671B6: Push A0 onto stack */
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[0]);

    /* ROM 0x1671B8: Call Obj_Free */
    Obj_Free(m, r->a[0]);

    /* ROM 0x1671BE: Restore stack pointer */
    r->a[7] += 4u;

    /* ROM 0x1671C2: Return */
    return 0;
}
