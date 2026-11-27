/* ROM 0x1673AA: spawn a child object (A0 = parent). Queues Obj_Spawn
 * (0x01057C) with fn 0x16742C, group 0, one group, arg 0, then re-arms the
 * parent with 0x1673D0. Obj_Spawn restores A0 and returns the new object in D0
 * (0xFFFFFFFF when the pool is full). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_1673AA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t parent = r->a[0];

    /* 0x1673AA move.l #0,-(SP) ; 0x1673B0 move.w #1,-(SP) ;
     * 0x1673B4 move.w #0,-(SP) ; 0x1673B8 move.l #0x16742c,-(SP) ;
     * 0x1673BE jsr 0x01057C ; 0x1673C4 lea (0xc,SP),SP */
    r->d[0] = Obj_Spawn(m, parent, 0x0016742Cu, 0u, 1u, 0u);

    wr32(m, parent + 0x00u, 0x001673D0u);            /* 0x1673C8 */
    return 0;
}
