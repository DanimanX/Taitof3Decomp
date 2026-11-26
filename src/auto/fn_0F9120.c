/* ROM 0x0F9120: explosion/spark emitter tick (A0 = object).
 * Scrolls +0x14/+0x18 by (A5+0x4224, A5+0x4228), decrements +0x21 and retires
 * via 0x0CEE74 when 0. Otherwise rolls Obj_Next4Way (0x0D0E3A) and if <= 0x46
 * spawns 0x0F917A (group 9, 2, arg 0) at (+0x14, +0x18). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0F9120(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = r->a[1];

    d0 = rd32(m, F3_A5 + 0x4224u);                      /* 0x0F9120 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0F9124 */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F9128 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x0F912C */

    uint8_t c = (uint8_t)(rd8(m, obj + 0x21u) - 1u);    /* 0x0F9130 */
    wr8(m, obj + 0x21u, c);
    if (c == 0u) {                                      /* 0x0F9134 */
        Obj_StartDying(m, obj);
        goto done;
    }

    uint16_t rnd = Obj_Next4Way(m);                     /* 0x0F913A */
    d0 = setw(d0, rnd);
    if (rnd <= 0x0046u) {                               /* 0x0F9140..0x0F9144 */
        uint32_t sp = Obj_Spawn(m, obj, 0x000F917Au, 9u, 2u, 0u); /* 0x0F915A */
        d0 = sp;
        if ((int32_t)sp >= 0) {                         /* 0x0F9164..0x0F9166 */
            a1 = sp;                                    /* 0x0F916A */
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));  /* 0x0F916C */
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));  /* 0x0F9172 */
        }
    }

done:
    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
