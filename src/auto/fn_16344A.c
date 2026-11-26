/* ROM 0x16344A: periodic spawner countdown tick (A0 = object).
 * When (+0x44 & 0x0F) == 0, spawns 0x1127C0 in group 0 (ngroups=1) with
 * arg = (D0.hi | (+0x44 >> 4)). Decrements +0x44; re-arms 0x16344A while >= 0,
 * else frees self via 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_16344A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t v = rd16(m, obj + 0x44u);                     /* 0x16344A */
    uint32_t d0 = setw(r->d[0], (uint16_t)(v & 0x000Fu));  /* 0x16344E */

    if ((uint16_t)d0 == 0u) {                              /* 0x163452 */
        d0 = setw(d0, (uint16_t)((int16_t)v >> 4));        /* 0x163456..0x16345A */
        d0 = Obj_Spawn(m, obj, 0x001127C0u, 0u, 1u, d0);   /* 0x16345C..0x163472 */
    }

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x44u) - 1u);  /* 0x163476 */
    wr16(m, obj + 0x44u, cnt);
    r->d[0] = d0;
    if ((int16_t)cnt >= 0) {                               /* 0x16347A */
        wr32(m, obj, 0x0016344Au);                         /* 0x163442 */
        return 0;
    }

    Obj_Free(m, obj);                                      /* 0x16347C..0x163484 */
    return 0;
}
