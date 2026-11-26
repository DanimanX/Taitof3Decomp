/* ROM 0x102010: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x102010 */
int Fn_102010(const f3mem_t *m, f3regs_t *r) {
    /* 102010 lea (0x66ea,PC),A1 */
    r->a[1] = 0x102014u + 0x66ea;
    /* 102014 jsr 0x000cede4.l */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x10201Au);
    Obj_InitMoverB(m, r->a[0], r->a[1]);
    r->a[7] += 4;
    /* 10201A movea.l (0x8,A0),A1 */
    r->a[1] = rd32(m, r->a[0] + 8);
    /* 10201E move.l (0x14,A1),(0x14,A0) */
    wr32(m, r->a[0] + 0x14, rd32(m, r->a[1] + 0x14));
    /* 102024 addi.w #0x10,(0x14,A0) */
    {
        uint16_t v = (uint16_t)(rd16(m, r->a[0] + 0x14) + 0x10);
        wr16(m, r->a[0] + 0x14, v);
    }
    /* 10202A move.l (0x18,A1),(0x18,A0) */
    wr32(m, r->a[0] + 0x18, rd32(m, r->a[1] + 0x18));
    /* 102030 addi.w #0x60,(0x18,A0) */
    {
        uint16_t v = (uint16_t)(rd16(m, r->a[0] + 0x18) + 0x60);
        wr16(m, r->a[0] + 0x18, v);
    }
    /* 102036 move.b #-0x38,(0x21,A0) */
    wr8(m, r->a[0] + 0x21, 0xC8);
    /* 10203C move.b #0x1,(0x23,A0) */
    wr8(m, r->a[0] + 0x23, 0x01);
    /* 102042 move.l #0x10204a,(A0) */
    wr32(m, r->a[0], 0x10204a);
    /* 102048 rts */
    return 0;
}
