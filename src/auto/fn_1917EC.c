/* ROM 0x1917EC: boss sub-spawner burst tick (A0 = object).
 * Decrements A5+0x06; while > 0 re-arms 0x1917EC. When <= 0, spawns
 * 0x191840 (group 6, ngroups 1, arg = (posX << 16) | posY) and decrements
 * +0x22; while > 0 reloads A5+0x06 = Obj_RandRange(5) + 5 and re-arms
 * 0x1917EC, else sets +0x06 = 2 and transitions to 0x19182C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

int Fn_1917EC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t timer = (uint16_t)(rd16(m, F3_A5 + 0x0006u) - 1u); /* 0x1917EC */
    wr16(m, F3_A5 + 0x0006u, timer);
    if ((int16_t)timer > 0) {                                   /* 0x1917F0 */
        wr32(m, obj, 0x001917ECu);                              /* 0x1917E4 */
        return 0;
    }

    uint32_t d1 = ((uint32_t)rd16(m, obj + 0x14u) << 16)
                | (uint32_t)rd16(m, obj + 0x18u);               /* 0x1917F2..0x1917F8 */
    r->d[1] = d1;
    r->d[0] = Obj_Spawn(m, obj, 0x00191840u, 6u, 1u, d1);       /* 0x1917FC..0x191812 */

    uint16_t count = (uint16_t)(rd16(m, obj + 0x22u) - 1u);     /* 0x191816 */
    wr16(m, obj + 0x22u, count);
    if ((int16_t)count > 0) {                                   /* 0x19181A */
        uint16_t delay = (uint16_t)(Obj_RandRange(m, 5u) + 5u); /* 0x1917D6..0x1917DE */
        r->d[0] = (uint32_t)delay;
        wr16(m, F3_A5 + 0x0006u, delay);                        /* 0x1917E0 */
        wr32(m, obj, 0x001917ECu);                              /* 0x1917E4 */
        return 0;
    }

    wr16(m, obj + 0x06u, 2u);                                   /* 0x19181C (0x191822 beq folded) */
    wr32(m, obj, 0x0019182Cu);                                  /* 0x191824 */
    return 0;
}
