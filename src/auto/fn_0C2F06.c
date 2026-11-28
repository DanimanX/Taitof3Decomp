/* ROM 0x0C2F06: short delay then free (A0 = object).
 * +0x06 counts down one per call; while it stays > 0 the tick re-arms
 * 0x0C2F06 (thunk 0x0C2EFE); on expiry it sets A5+0x08F6 = 0xFF (the
 * "wave finished" byte) and frees the object (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_0C2F06(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x0C2F06 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                                  /* 0x0C2F0A bgt */
        wr32(m, obj, 0x000C2F06u);                                  /* 0x0C2EFE */
        return 0;
    }

    wr8(m, F3_A5 + 0x08F6u, 0xFFu);                                 /* 0x0C2F0C */
    wr32(m, r->a[7] - 4u, obj);                                     /* 0x0C2F12 move.l A0,-(SP) */
    Obj_Free(m, obj);                                               /* 0x0C2F14 -> 0x0105EC */
    return 0;                                                       /* 0x0C2F1E rts */
}
