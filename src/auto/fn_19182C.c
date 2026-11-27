/* ROM 0x19182C: countdown ticker: decrements +0x06.w and re-arms 0x191824
 * while it stays positive, else frees the object (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_19182C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x19182C subq.w #1 */

    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {                                        /* 0x191830 bgt */
        wr32(m, obj, 0x0019182Cu);                      /* 0x191824 thunk stores 0x19182C */
        return 0;
    }
    r->a[7] -= 4;                                       /* 0x191832 */
    wr32(m, r->a[7], obj);
    Obj_Free(m, obj);                                   /* 0x191834 -> 0x0105EC */
    r->a[7] += 4;                                       /* 0x19183A */
    return 0;
}
