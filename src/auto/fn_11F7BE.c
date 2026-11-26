/* ROM 0x11F7BE: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_11F7BE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F7BE: movea.l (0x8,A0),A1 */
    r->a[1] = rd32(m, r->a[0] + 8);
    
    /* ROM 0x11F7C2: tst.b (0x59,A1) */
    uint8_t t = rd8(m, r->a[1] + 0x59);
    
    /* ROM 0x11F7C6: bne.w 0x0011f7e2 */
    if (t != 0) {
        /* ROM 0x11F7E2: move.l A0,-(SP) */
        r->a[7] -= 4;
        wr32(m, r->a[7], r->a[0]);
        
        /* ROM 0x11F7E4: jsr 0x000105ec.l */
        Obj_Free(m, r->a[0]);
        
        /* ROM 0x11F7EA: lea (0x4,SP),SP */
        r->a[7] += 4;
        
        /* ROM 0x11F7EE: rts */
        return 0;
    }
    
    /* ROM 0x11F7CA: moveq 0x0,D0 */
    r->d[0] = 0;
    
    /* ROM 0x11F7CC: move.b (0x30f2,A5),D0b */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, F3_A5 + 0x30f2);
    
    /* ROM 0x11F7D0: beq.b 0x0011f7b6 */
    if (r->d[0] == 0) {
        /* ROM 0x11F7B6: move.l #0x11f7be,(A0) */
        wr32(m, r->a[0], 0x0011F7BEu);
        /* ROM 0x11F7BC: rts */
        return 0;
    }
    
    /* ROM 0x11F7D2: sf (0x30f2,A5) */
    wr8(m, F3_A5 + 0x30f2, 0);
    
    /* ROM 0x11F7D6: lea (0x18,PC),A1 */
    r->a[1] = 0x0011F7F0u;
    
    /* ROM 0x11F7DA: nop */
    
    /* ROM 0x11F7DC: movea.l (-0x4,A1,D0w*0x4),A1 */
    r->a[1] = rd32(m, r->a[1] - 4);
    
    /* ROM 0x11F7E0: jmp (A1) */
    typedef int (*fn_t)(const f3mem_t*, f3regs_t*);
    return ((fn_t)(uintptr_t)r->a[1])(m, r);
}
