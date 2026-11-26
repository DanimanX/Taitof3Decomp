/* ROM 0x179D92: particle/explosion object initializer (A0 = object).
 * Frees self via 0x0105EC if (int16_t)A5+0x4B24 > 0x1E. Otherwise increments
 * A5+0x4B24, sets (+0x50, +0x54) from parent (+0x08) + signed (+0x0C.w, +0x0E.w)
 * via 0x179AE4, loads anim list 0x179B3A and runs its 0x179A5C init entry,
 * clears +0x1C and +0x1E, and transitions to 0x179C26. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"

int Fn_179D92(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x001E) {       /* 0x179D92..0x179D98 */
        Obj_Free(m, obj);                                   /* 0x179D18..0x179D24 */
        return 0;
    }

    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u)); /* 0x179D9C */

    /* ROM 0x179AE4: (+0x50, +0x54) = parent (+0x08) + signed (+0x0C.w, +0x0E.w) */
    uint32_t a4 = rd32(m, obj + 0x08u);                     /* 0x179AE4 */
    uint32_t d0 = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x0Cu); /* 0x179AE8 */
    uint32_t d1 = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x0Eu); /* 0x179AEE */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + rd16(m, a4 + 0x50u)); /* 0x179AF4 */
    d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 + rd16(m, a4 + 0x54u)); /* 0x179AF8 */
    wr16(m, obj + 0x50u, (uint16_t)d0);                     /* 0x179AFC */
    wr16(m, obj + 0x54u, (uint16_t)d1);                     /* 0x179B00 */

    uint32_t a1 = 0x00179B3Au;                              /* 0x179DA4 */
    uint16_t idx = rd16(m, a1);                             /* 0x179DAA */
    a1 += 2u;
    d0 = (d0 & 0xFFFF0000u) | idx;
    wr16(m, obj + 0x40u, idx);                              /* 0x179DAC */
    wr32(m, obj + 0x3Au, a1);                               /* 0x179DB0 */

    /* ROM 0x179DB4..0x179DBE: dispatch init half at 0x179A5C[idx * 8] */
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;
    if (idx == 0u)
        a1 += 6u;                                           /* 0x179550 reads +0x3E and +0x10 */
    else
        a1 = rd32(m, 0x00179A5Cu + (uint32_t)idx * 8u);

    wr16(m, obj + 0x1Cu, 0u);                               /* 0x179DC0 */
    wr16(m, obj + 0x1Eu, 0u);                               /* 0x179DC4 */
    wr32(m, obj, 0x00179C26u);                              /* 0x179DC8 -> 0x179C1E */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
