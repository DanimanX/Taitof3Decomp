/* ROM 0x18CAF0: countdown tick of an object with a timer at +0x06 (word).
 * Decrements (+0x06).w one byte code of progress per call; while the result
 * stays positive it re-arms 0x18CAF0 (0x18CAE8), otherwise it frees the object
 * via Obj_Free (0x0105EC) and rts. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_18CAF0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t w = (uint16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x18CAF0: subq.w #1,(0x6,A0) */

    wr16(m, obj + 0x06u, w);
    if ((int16_t)w > 0) {                            /* 0x18CAF4: bgt 0x18CAE8 */
        wr32(m, obj, 0x0018CAF0u);                   /* 0x18CAE8 */
        return 0;
    }
    Obj_Free(m, obj);                                /* 0x18CAF6..0x18CB02 */
    return 0;
}
