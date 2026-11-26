/* ROM 0x16743A: fade GFX priority/fill words at 0x626200 + (A5-0x5D56) from
 * +0x44 countdown (A0 = object). Computes RGB-like nibble mask from +0x44>>5 + 4
 * and 4, writes 232 words, decrements +0x44, and frees self when < 0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_16743A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t d0w = (int16_t)(((int16_t)rd16(m, obj + 0x44u) >> 5) + 4); /* 0x16743A..0x167440 */
    uint16_t d1w = 4u;                                                 /* 0x167442 */

    /* Saved by movem.l {D0-D7,A0},-(SP) at 0x16744A and restored at 0x1674A2 */
    uint16_t ret_d0w = (uint16_t)d0w;
    uint16_t ret_d1w = d1w;

    uint32_t gfx = (uint32_t)(0x00626200u + (int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u)); /* 0x16744E..0x167454 */
    d1w &= 0x000Fu;                                              /* 0x167458 */
    if ((int16_t)d1w > 8)                                        /* 0x16745C..0x167460 */
        d1w = 8u;
    uint16_t u0w = (uint16_t)d0w & 0x000Fu;                      /* 0x167468 */
    if ((int16_t)u0w > 8)                                        /* 0x16746C..0x167470 */
        u0w = 8u;

    /* 0x167478..0x167490 */
    uint16_t v1 = (uint16_t)(((d1w << 8) + (uint16_t)(8u - d1w)) << 4);
    uint16_t v0 = (uint16_t)((u0w << 8) + (uint16_t)(8u - u0w));
    uint16_t fill = (uint16_t)~(v1 | v0);

    for (int i = 0xE7; i >= 0; i--) {                            /* 0x167492..0x16749E */
        uint16_t off = (uint16_t)((uint16_t)i * 2u);
        wr16(m, gfx + off, fill);
    }

    int16_t cnt = (int16_t)(rd16(m, obj + 0x44u) - 1u);          /* 0x1674AA */
    wr16(m, obj + 0x44u, (uint16_t)cnt);
    if (cnt >= 0) {                                              /* 0x1674AE bge */
        wr32(m, obj + 0x00u, 0x0016743Au);                       /* 0x167432 */
    } else {
        Obj_Free(m, obj);                                        /* 0x1674B2 */
    }

    r->d[0] = setw(r->d[0], ret_d0w);
    r->d[1] = setw(r->d[1], ret_d1w);
    return 0;
}
