/* ROM 0x0C0630: object init. Clears the per-frame fields (0x0CBD86), sets the
 * script pointer 0x000C0732 at +0x5C, the longs 0x00800000/0x00700000 at
 * +0x14/+0x18, the word 0x0084 at +0x60, then re-arms with state 0x0C065C. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_0C0630(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);             /* 0x0C0630 jsr 0x0CBD86 */
    wr32(m, obj + 0x5Cu, 0x000C0732u);   /* 0x0C0636 */
    wr32(m, obj + 0x14u, 0x00800000u);   /* 0x0C063E */
    wr32(m, obj + 0x18u, 0x00700000u);   /* 0x0C0646 */
    wr16(m, obj + 0x60u, 0x0084u);       /* 0x0C064E */
    wr32(m, obj + 0x00u, 0x000C065Cu);   /* 0x0C0654 -> 0x0C065C rts */
    return 0;
}
