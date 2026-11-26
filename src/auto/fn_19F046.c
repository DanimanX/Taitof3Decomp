/* ROM 0x19F046: parent-tracking child spawner & countdown tick (A0 = object).
 * Copies parent (+0x08) position (+0x50, +0x54) into self (+0x50, +0x54),
 * spawns child 0x19F0CE (group 9, 2 groups, arg 0) on even frames ((A5+0x878)&1
 * == 0), decrements +0x06 and re-arms 0x19F046 while > 0, else frees self via
 * 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_19F046(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x19F046 */
    r->a[4] = a4;

    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));           /* 0x19F04A */
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));           /* 0x19F050 */

    uint16_t d0w = rd16(m, F3_A5 + 0x0878u) & 0x0001u;   /* 0x19F056..0x19F05E */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
    if (d0w == 0u)                                       /* 0x19F060 */
        r->d[0] = Obj_Spawn(m, obj, 0x0019F0CEu, 9u, 2u, 0u); /* 0x19F064..0x19F07E */

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x19F082 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0)                                         /* 0x19F086 */
        wr32(m, obj, 0x0019F046u);                       /* 0x19F03E */
    else
        Obj_Free(m, obj);                                /* 0x19F088..0x19F090 */

    return 0;
}
