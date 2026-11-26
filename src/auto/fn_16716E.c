/* ROM 0x16716E: scroll-accumulator countdown tick (A0 = object).
 * Adds 0x240 to A5+0x4240 and 0x1A0 to A5+0x4230, decrements +0x44, and
 * re-arms 0x16716E while >= 0; otherwise frees the object via 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_16716E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x4240u, rd32(m, F3_A5 + 0x4240u) + 0x00000240u); /* 0x16716E */
    wr32(m, F3_A5 + 0x4230u, rd32(m, F3_A5 + 0x4230u) + 0x000001A0u); /* 0x167176 */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x44u) - 1u);               /* 0x16717E */
    wr16(m, obj + 0x44u, cnt);
    if ((int16_t)cnt >= 0) {                                            /* 0x167182 -> 0x167166 */
        wr32(m, obj, 0x0016716Eu);
        return 0;
    }

    Obj_Free(m, obj);                                                   /* 0x167184..0x16718C */
    return 0;                                                           /* 0x167190 */
}
