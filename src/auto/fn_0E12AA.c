/* ROM 0x0E12AA: object initializer (A0 = object).
 * Spawns two children (0x0E186A and 0x0E18CA, group 5, count 2); if the first
 * fails, dies (0x0CEE74). Seeds from 0x0E08FC (0x0CEC4C), picks bit 0 of
 * Obj_Next4Way (0x0D0E3A) to select script (0x0E14D0[idx] -> 0x0CF1D8) and
 * behaviour (0x0E1616[idx] -> 0x0D0678), sets +0x06 = 1 and arms 0x0E1328. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E12AA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0E12AA: spawn first child 0x0E186A */
    {
        uint32_t c1 = Obj_Spawn(m, obj, 0x000E186Au, 5u, 2u, 0u);
        r->d[0] = c1;
        if ((int32_t)c1 < 0) {
            Obj_StartDying(m, obj);                           /* 0x0E12C8 -> 0x0CEE74 */
            return 0;
        }
    }

    /* 0x0E12CE: spawn second child 0x0E18CA */
    r->d[0] = Obj_Spawn(m, obj, 0x000E18CAu, 5u, 2u, 0u);

    /* 0x0E12EA: seed from 0x0E08FC */
    Obj_SeedFromParams(m, obj, 0x000E08FCu);                  /* 0x0CEC4C */

    /* 0x0E12F4: random bit 0 selects script & behaviour */
    {
        uint16_t idx = (uint16_t)(Obj_Next4Way(m) & 1u);      /* 0x0D0E3A */
        uint32_t scr = rd32(m, 0x000E14D0u + (uint32_t)idx * 4u);
        Obj_StartScript(m, obj, scr);                         /* 0x0E1306 -> 0x0CF1D8 */
        uint32_t beh = rd32(m, 0x000E1616u + (uint32_t)idx * 4u);
        Obj_StartBehaviour(m, obj, beh);                      /* 0x0E1314 -> 0x0D0678 */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, beh + 2u);
        r->a[1] = beh + 8u;
    }

    wr8(m, obj + 0x06u, 1u);                                  /* 0x0E131A */
    wr32(m, obj, 0x000E1328u);                                /* 0x0E1320 */
    return 0;
}
