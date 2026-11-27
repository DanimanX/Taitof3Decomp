/* ROM 0x11291E: object init variant (A0 = object). Resets the common fields
 * via 0x0CBD86, copies the 4-long record at 0x112956 into (+0x10, +0x14,
 * +0x18, +0x44) and re-arms the object with 0x1129BC.
 * 0x112996 is a shared body: the sibling entries 0x112928 / 0x112932 /
 * 0x11293C reach it with A1 = 0x112966 / 0x112976 / 0x112986; only the
 * 0x11291E entry (A1 = 0x112956) is part of this routine. There is no
 * conditional branch: 0x112924 always jumps over the sibling lea/bra pairs. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_11291E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    /* 0x11291E: lea 0x36(PC),A1 (ext word at 0x112920 -> 0x112956),
     * 0x112924: bra.w 0x112996 */
    uint32_t a1 = 0x00112956u;

    Obj_ResetFields(m, obj);                             /* 0x11299A -> 0x0CBD86 */

    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;         /* 0x1129A4 */
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;         /* 0x1129A8 */
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;         /* 0x1129AC */
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;         /* 0x1129B0 */
    r->a[1] = a1;

    wr32(m, obj + 0x00u, 0x001129BCu);                   /* 0x1129B4 */
    return 0;
}
