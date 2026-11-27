/* ROM 0x19832C: object step (A0 = object). Decrements (+0x06).w; while it
 * stays positive the object re-arms itself with 0x19832C, otherwise it is
 * freed via Obj_Free (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_19832C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x19832C subq.w */
    wr16(m, obj + 0x06u, (uint16_t)n);

    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0019832Cu);               /* 0x198330 bgt -> 0x198324 */
        return 0;
    }

    Obj_Free(m, obj);                                   /* 0x198332 push A0 ; jsr 0x0105EC */
    return 0;
}
