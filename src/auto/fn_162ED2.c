/* ROM 0x162ED2: line-scroll table init for one stage. Sets the two flag
 * bytes at A5+0x4284/A5+0x4286 to 4, installs the four script pointers at
 * 0x40C264 and 0x40C274 (0x1637B4/0x16383C and 0x1637FC/0x163858), stores
 * the screen bounds 0x33F/0x2BF at A5+0x425C/A5+0x4260, then clears the
 * scroll pairs at A5+0x4204/A5-0x5DA8, A5+0x4214/A5-0x5D98 (with D1 =
 * 0x100000) and sets A5+0x4224 = 0x33E00 and A5+0x4234 = 0x15F00. Finally it
 * flags word A0+6 = 0x100 and re-arms the caller object with 0x162FBA.
 * A1.w ends post-incremented by the pair store; A0 is saved and restored. */
#include "fn_auto.h"
#include "../objects/obj_common.h"      /* Mem_WritePair64 (0x16AB2C) */
#include "../objects/obj_shared_ext.h"  /* Obj_16AB26 (0x16AB26) */

int Fn_162ED2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t a1 = r->a[1];

    /* 0x162ED6..0x162EEC: 0x16AB58 (store D0.b at A0) twice */
    wr8(m, F3_A5 + 0x4284u, 0x04u);
    wr8(m, F3_A5 + 0x4286u, 0x04u);

    /* 0x162EF2..0x162F08: pushed 0x1637B4 then 0x16383C pop into D1, D0;
     * 0x16AB5C stores (D0, D1) at 0x40C264 and advances A0 past them */
    wr32(m, 0x0040C264u, 0x001637B4u);
    wr32(m, 0x0040C268u, 0x0016383Cu);

    /* 0x162F0E..0x162F24 */
    wr32(m, 0x0040C274u, 0x001637FCu);
    wr32(m, 0x0040C278u, 0x00163858u);

    /* 0x162F2A..0x162F30 */
    wr16(m, F3_A5 + 0x425Cu, 0x033Fu);
    wr16(m, F3_A5 + 0x4260u, 0x02BFu);

    /* 0x162F36..0x162F4E: D0 = D1 = 0; 0x16AB2C writes the pair to A0 and A1 */
    Mem_WritePair64(m, F3_A5 + 0x4204u, F3_A5 - 0x5DA8u, 0u, 0u);

    /* 0x162F54..0x162F6C: D0 = 0, D1 = 0x100000. The callee's (A1)+ leaves
     * A1 four past the pair's base. */
    Mem_WritePair64(m, F3_A5 + 0x4214u, F3_A5 - 0x5D98u, 0u, 0x00100000u);
    a1 = F3_A5 - 0x5D98u + 4u;

    /* 0x162F72..0x162F86: D0 = 0x33E00, D1 = 0 */
    Obj_16AB26(m, F3_A5 + 0x4224u, 0x00033E00u, 0u);

    /* 0x162F8C..0x162FA0: D0 = 0x15F00, D1 = 0 */
    Obj_16AB26(m, F3_A5 + 0x4234u, 0x00015F00u, 0u);

    /* 0x162FA6 movem.l (SP)+,A0 restores A0; 0x162FAA move.w #0x100,(6,A0)
     * leaves Z clear, so the beq at 0x162FB0 to 0x162FC0 is never taken
     * (that block is a separate entry point, not reachable from here). */
    wr16(m, a0 + 6u, 0x0100u);
    wr32(m, a0, 0x00162FBAu);                     /* 0x162FB2 move.l #0x162fba,(A0) */

    r->d[0] = 0x00015F00u;                        /* last pop D0 at 0x162F9E */
    r->d[1] = 0x00000000u;                        /* last pop D1 at 0x162F9C */
    r->a[0] = a0;                                 /* restored by movem.l (SP)+ */
    r->a[1] = a1;                                 /* 0x16AB2C (A1)+ at 0x162F6C */
    return 0;                                     /* 0x162FB8 rts */
}
