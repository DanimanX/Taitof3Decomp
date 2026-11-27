/* ROM 0x13272E: reset the per-frame fields, set frame +0x1D = 4,
 * speed/scale words +0x2C = -0x200 and +0x30 = -0x1000, clear the flip byte
 * +0x22 and hand the object to the next state 0x132752. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_13272E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);            /* ROM 0x13272E: jsr 0x000CBD86 */
    wr8(m, obj + 0x1Du, 0x04u);         /* ROM 0x132734: move.b #4,(0x1D,A0) */
    wr16(m, obj + 0x2Cu, 0xFE00u);      /* ROM 0x13273A: move.w #-0x200,(0x2C,A0) */
    wr16(m, obj + 0x30u, 0xF000u);      /* ROM 0x132740: move.w #-0x1000,(0x30,A0) */
    wr8(m, obj + 0x22u, 0x00u);         /* ROM 0x132746: sf (0x22,A0) */
    wr32(m, obj, 0x00132752u);          /* ROM 0x13274A: move.l #0x132752,(A0) */
    return 0;                           /* ROM 0x132750 rts */
}
