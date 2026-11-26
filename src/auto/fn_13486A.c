/* ROM 0x13486A: attached hitbox proxy tick (A0 = object, A0+0x08 = parent).
 * Mirrors parent+0x59 into +0x59 and frees self if non-zero. Copies parent
 * (+0x14, +0x18), loads hitbox (+0x2C, +0x30, +0x2E, +0x32) from (+0x0C)[parent+0x63],
 * mirrors X bounds around parent's sprite width << 4 when parent+0x62 != 0,
 * appends to collision lists (0x0CBDA4) when A5+0x30F3 == 0 and +0x30.l != 0,
 * and re-arms 0x13486A. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CBDA4: append obj to both A5+0x8FE and A5+0x906. */
static uint32_t sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    return a1;
}

int Fn_13486A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);             /* 0x13486A */
    r->a[1] = a1;

    uint8_t dead = rd8(m, a1 + 0x59u);              /* 0x13486E */
    wr8(m, obj + 0x59u, dead);
    if (dead != 0u) {                               /* 0x134874 -> 0x1348EA */
        Obj_Free(m, obj);
        return 0;
    }

    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));      /* 0x13487C */
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));      /* 0x134882 */

    uint32_t a2 = rd32(m, obj + 0x0Cu);             /* 0x134888 */
    uint32_t d0 = rd8(m, a1 + 0x63u);               /* 0x13488C..0x13488E */
    r->d[0] = d0;
    a2 += d0 * 8u;                                  /* 0x134892 */
    wr16(m, obj + 0x2Cu, rd16(m, a2));              /* 0x134896 */
    wr16(m, obj + 0x30u, rd16(m, a2 + 2u));         /* 0x13489A */
    wr16(m, obj + 0x2Eu, rd16(m, a2 + 4u));         /* 0x13489E */
    wr16(m, obj + 0x32u, rd16(m, a2 + 6u));         /* 0x1348A2 */

    if (rd8(m, a1 + 0x62u) != 0u) {                 /* 0x1348A6..0x1348AA */
        a1 = rd32(m, a1 + 0x10u);                   /* 0x1348AE */
        r->a[1] = a1;
        uint16_t d2w = (uint16_t)(rd16(m, a1 + 0x02u) << 4); /* 0x1348B2..0x1348B6 */
        uint16_t d0w = (uint16_t)((uint16_t)(0u - d0) - d2w); /* 0x1348B8..0x1348BA */
        r->d[0] = (d0 & 0xFFFF0000u) | d0w;
        d2w = (uint16_t)(d2w - 1u);                 /* 0x1348BC */
        uint16_t d3w = d2w;                         /* 0x1348BE */
        d2w = (uint16_t)(d2w - rd16(m, obj + 0x2Eu)); /* 0x1348C0 */
        d3w = (uint16_t)(d3w - rd16(m, obj + 0x2Cu)); /* 0x1348C4 */
        wr16(m, obj + 0x2Cu, d2w);                  /* 0x1348C8 */
        wr16(m, obj + 0x2Eu, d3w);                  /* 0x1348CC */
    }

    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x30u) != 0u) /* 0x1348D0..0x1348DC */
        r->a[1] = sub_0CBDA4(m, obj);               /* 0x1348E0 */

    wr32(m, obj, 0x0013486Au);                      /* 0x1348E6 -> 0x134862 */
    return 0;
}
