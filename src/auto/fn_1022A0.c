/* ROM 0x1022A0: parent-attached effect/emitter tick (A0 = object).
 * Dies via 0x0CEE74 if parent (+0x08)->+0x06.b == 0, or if +0x21.b decrements
 * to 0 after subtracting A5+0x4228 from +0x3E.l. Rolls 0x0D0E3A; if <= 0x0F,
 * spawns child 0x102310 (group 5, count 2, arg 0) offset (+0x30, +0x08) from
 * parent (+0x14, +0x18) with +0x10 = parent +0x3E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

int Fn_1022A0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x1022A0 */
    r->a[4] = a4;

    if (rd8(m, a4 + 0x06u) == 0u) {                      /* 0x1022A4 */
        Obj_StartDying(m, obj);                          /* 0x1022A8 -> 0x0CEE74 */
        return 0;
    }

    uint32_t d0 = rd32(m, F3_A5 + 0x4228u);              /* 0x1022AE */
    r->d[0] = d0;
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);     /* 0x1022B2 */

    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x21u) - 1u);   /* 0x1022B6 */
    wr8(m, obj + 0x21u, cnt);
    if (cnt == 0u) {                                     /* 0x1022BA -> 0x0CEE74 */
        Obj_StartDying(m, obj);
        return 0;
    }

    uint16_t rng = Rng_Mix1D60(m);                       /* 0x1022C0 -> 0x0D0E3A */
    r->d[0] = (d0 & 0xFFFF0000u) | rng;
    if (rng > 0x000Fu)                                   /* 0x1022C6..0x1022CA bhi */
        return 0;

    uint32_t child = Obj_Spawn(m, obj, 0x00102310u, 5u, 2u, 0u); /* 0x1022CE..0x1022E6 */
    r->d[0] = child;
    if ((int32_t)child < 0)                              /* 0x1022EA..0x1022EC */
        return 0;

    r->a[1] = child;                                     /* 0x1022F0 */
    wr32(m, child + 0x14u, rd32(m, a4 + 0x14u));         /* 0x1022F2 */
    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0030u)); /* 0x1022F8 */
    wr32(m, child + 0x18u, rd32(m, a4 + 0x18u));         /* 0x1022FE */
    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 8u)); /* 0x102304 */
    wr32(m, child + 0x10u, rd32(m, obj + 0x3Eu));        /* 0x102308 */
    return 0;
}
