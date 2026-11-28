/* ROM 0x1312D2: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_1312D2(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1312D2: Load 32-bit value from A0+0x48 into D0 */
    r->d[0] = rd32(m, r->a[0] + 0x48u);
    
    /* ROM 0x1312D6: Branch to 0x1312E0 if D0 is zero */
    if (r->d[0] == 0)
        goto skip_clear;
        
    /* ROM 0x1312DA: Move D0 to A1 */
    r->a[1] = r->d[0];
    
    /* ROM 0x1312DC: Clear 32-bit value at A1+0x44 */
    wr32(m, r->a[1] + 0x44u, 0u);
    
skip_clear:
    /* ROM 0x1312E0: Push A0 onto stack */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    
    /* ROM 0x1312E2: Call Obj_Free(m, A0) */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x1312E8u);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4;
    
    /* ROM 0x1312E8: Adjust stack pointer */
    r->a[7] += 4;
    
    /* ROM 0x1312EC: Return */
    return 0;
}
