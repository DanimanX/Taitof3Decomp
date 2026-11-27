#include "fn_auto.h"
/* ROM 0x12C126: Reset the object and copy the parent's bounds before starting its timer. */
#include "../objects/obj_common.h"

int Fn_12C126(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, r->a[7] - 4u, 0x12C12Cu);
    Obj_ResetFields(m, obj); /* 0x12C126 */
    r->a[1] = rd32(m, obj + 8u);
    wr32(m, obj + 0x2Cu, rd32(m, r->a[1] + 0x2Cu));
    wr32(m, obj + 0x30u, rd32(m, r->a[1] + 0x30u));
    wr16(m, obj + 0x28u, 1u);
    wr16(m, obj + 0x24u, 1u);
    wr8(m, obj + 0x1Du, 6u);
    wr16(m, obj + 6u, 0x32u);
    wr32(m, obj, 0x12C15Cu); /* 0x12C154 */
    r->a[7] += 4u;
    return 0;
}
