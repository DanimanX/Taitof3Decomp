/* ROM 0x19C696: countdown on obj+6; on expiry free the object, otherwise
 * re-arm the entry with itself (0x19C696). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_19C696(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t v = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* ROM 0x19C696 subq.w #1,(0x6,A0) */

    wr16(m, obj + 0x06u, v);
    if ((int16_t)v > 0) {                                  /* ROM 0x19C69A bgt.b 0x19C68E */
        wr32(m, obj, 0x0019C696u);                         /* ROM 0x19C68E move.l #0x19c696,(A0) */
        return 0;
    }

    Obj_Free(m, obj);                                      /* ROM 0x19C69C jsr 0x0105EC */
    return 0;                                              /* ROM 0x19C6A8 rts */
}
