/* ROM 0x12D570: spawn 2 children of routine 0x13272E in group 5 with arg 0
 * (parent = A0), leave the new slot in D0 and re-arm the caller with 0x12D594. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_12D570(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x12D570 pea #0 ; move.w #2,-(SP) ; move.w #5,-(SP) ; move.l #0x13272E,-(SP)
     * ; jsr 0x1057C ; lea (0xC,SP),SP  -> group 5, 2 slots, arg 0 */
    r->d[0] = Obj_Spawn(m, obj, 0x0013272Eu, 5, 2, 0);
    wr32(m, obj + 0x00u, 0x0012D594u); /* 0x12D58C -> 0x12D594 rts */
    return 0;
}
