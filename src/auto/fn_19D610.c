/* ROM 0x19D610: boss-controller initialiser (A0 = object). Clears the common
 * fields via Obj_17A7FA (0x17A7FA), copies the global 32-bit latch at
 * A5+0x1D28 (0x409D28) into +0x4C, reads the animation index word at 0x19D76E,
 * latches it to +0x40 with the following record pointer 0x19D770 in +0x3A and
 * runs that index's init half through the 0x179A5C dispatch; finally re-arms
 * the object with 0x19D640.
 *
 * The word at ROM 0x19D76E is 0x0000, so the dispatch takes entry 0 of the
 * 0x179A5C table, whose init half is 0x179550 (translated inline below). */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_19D610(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                              /* 0x19D610 -> 0x17A7FA */
    wr32(m, obj + 0x4Cu, rd32(m, 0x409D28u));        /* 0x19D616 move.l (0x1d28,A5),(0x4c,A0) */

    /* 0x19D61C movea.l #0x19d76e,A1 ; 0x19D622 move.w (A1)+,D0w */
    uint16_t idx = rd16(m, 0x0019D76Eu);             /* == 0 for this ROM */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | idx;         /* D0.w = the table word */
    wr16(m, obj + 0x40u, idx);                       /* 0x19D624 */
    uint32_t a1 = 0x0019D770u;                       /* A1 after the post-increment */
    wr32(m, obj + 0x3Au, a1);                        /* 0x19D628 move.l A1,(0x3a,A0) */

    /* 0x19D62C lea (0x179a5c).l,A1 ; 0x19D632 movea.l (0x0,A1,D0w*0x8),A1 ;
     * 0x19D636 jsr (A1) with D0w = 0 -> init half 0x179550: */
    a1 = rd32(m, obj + 0x3Au);                       /* 0x179550 movea.l (0x3a,A0),A1 */
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;     /* 0x179554 */
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;     /* 0x179558 */
    r->a[1] = a1;                                    /* A1 = 0x19D776 at rts */

    wr32(m, obj + 0x00u, 0x0019D640u);               /* 0x19D638 */
    return 0;
}
