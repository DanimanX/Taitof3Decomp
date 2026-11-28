/* ROM 0x183EC6: short death/delay countdown (A0 = object).
 * +0x06 decremented by one per call; while it stays > 0 the tick re-arms
 * 0x183EC6 (thunk 0x183EBE), otherwise the object is freed (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_183EC6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x183EC6 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                                  /* 0x183ECA bgt */
        wr32(m, obj, 0x00183EC6u);                                  /* 0x183EBE */
        return 0;
    }

    wr32(m, r->a[7] - 4u, obj);                                     /* 0x183ECC move.l A0,-(SP) */
    Obj_Free(m, obj);                                               /* 0x183ECE -> 0x0105EC */
    return 0;                                                       /* 0x183ED8 rts */
}
