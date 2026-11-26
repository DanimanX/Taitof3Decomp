/* ROM 0x179F26: boss particle/spark initializer (A0 = object).
 * Frees self via 0x0105EC if A5+0x4B24 > 0x1E. Otherwise increments A5+0x4B24,
 * sets (+0x50, +0x54) from parent (+0x08) + signed (+0x0C.w, +0x0E.w) via
 * 0x179AE4, picks one of two anim lists via 0x179B06, installs script
 * 0x179F12, clears +0x4A/+0x1E/+0x1C, rolls random angle +0x20.b via
 * 0x1794A4(0x100), and transitions to 0x179C6E. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

int Fn_179F26(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x001E) {    /* 0x179F26..0x179F2C */
        Obj_Free(m, obj);                                /* 0x179D18..0x179D24 */
        return 0;
    }

    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u)); /* 0x179F30 */

    /* ROM 0x179AE4: (+0x50, +0x54) = parent (+0x08) + signed (+0x0C.w, +0x0E.w) */
    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x179AE4 */
    uint32_t d0 = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x0Cu); /* 0x179AE8 */
    uint32_t d1 = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x0Eu); /* 0x179AEE */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + rd16(m, a4 + 0x50u)); /* 0x179AF4 */
    d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 + rd16(m, a4 + 0x54u)); /* 0x179AF8 */
    wr16(m, obj + 0x50u, (uint16_t)d0);                  /* 0x179AFC */
    wr16(m, obj + 0x54u, (uint16_t)d1);                  /* 0x179B00 */

    /* ROM 0x179B06: both lists at 0x179B3A and 0x179B8C start with index 0 */
    if (Obj_AnimList179B06(m, obj) < 0)                  /* 0x179F38 */
        return -1;

    uint32_t a1 = 0x00179F12u;                           /* 0x179F3C */
    wr32(m, obj + 0x44u, a1);                            /* 0x179F42 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));              /* 0x179F46 */
    wr16(m, obj + 0x4Au, 0u);                            /* 0x179F4C */
    wr16(m, obj + 0x1Eu, 0u);                            /* 0x179F50 */
    wr16(m, obj + 0x1Cu, 0u);                            /* 0x179F54 */

    d0 = (uint32_t)Obj_RandRange(m, 0x0100u);            /* 0x179F58..0x179F5E */
    wr8(m, obj + 0x20u, (uint8_t)d0);                    /* 0x179F64 */
    wr32(m, obj, 0x00179C6Eu);                           /* 0x179F68 -> 0x179C66 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
