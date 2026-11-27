/* ROM 0x195BF8: object step (A0 = object). Decrements (+0x06).w; while it
 * stays positive the object re-arms itself with 0x195BF8, otherwise it frees
 * itself (jsr 0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_195BF8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x195BF8 */
    wr16(m, obj + 0x06u, (uint16_t)n);

    if (n > 0)
        wr32(m, obj + 0x00u, 0x00195BF8u);               /* 0x195BF0 (bgt) */
    else
        Obj_Free(m, obj);                                /* 0x195BFE -> 0x0105EC */
    return 0;
}
