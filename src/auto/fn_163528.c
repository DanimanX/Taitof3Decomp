/* ROM 0x163528: line-RAM fade-in/out ramp tick (A0 = object).
 * Computes fade level 8 - (+0x44 >> 4) clamped to [0..8], packs RGB fade word
 * with level 4 high nibble, fills 0xE8 words at 0x626200 + (int16_t)(A5-0x5D56),
 * decrements +0x44, and re-arms 0x163528 while >= 0, else frees self. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_163528(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t level = (int16_t)(8 - ((int16_t)rd16(m, obj + 0x44u) >> 4)); /* 0x163528..0x163530 */
    uint16_t d0w = (uint16_t)level;
    uint16_t d1w = 0x0004u;                         /* 0x163532 */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;

    int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x163544 */
    uint32_t lram = 0x00626200u + (uint32_t)(int32_t)off; /* 0x16353E */

    uint16_t c1 = (uint16_t)(d1w & 0x000Fu);        /* 0x163548 */
    if ((int16_t)c1 > 8)                            /* 0x16354C..0x163550 */
        c1 = 8u;
    uint16_t c0 = (uint16_t)(d0w & 0x000Fu);        /* 0x163558 */
    if ((int16_t)c0 > 8)                            /* 0x16355C..0x163560 */
        c0 = 8u;

    uint16_t hi = (uint16_t)(((c1 << 8) + (uint16_t)(8u - c1)) << 4); /* 0x163568..0x163572 */
    uint16_t lo = (uint16_t)((c0 << 8) + (uint16_t)(8u - c0)); /* 0x163574..0x16357C */
    uint16_t fill = (uint16_t)~(hi | lo);           /* 0x16357E..0x163580 */

    for (int16_t d7 = 0x00E7; d7 >= 0; d7--)        /* 0x163582..0x16358E */
        wr16(m, lram + (uint32_t)(uint16_t)d7 * 2u, fill);

    uint16_t rem = (uint16_t)(rd16(m, obj + 0x44u) - 1u); /* 0x16359A */
    wr16(m, obj + 0x44u, rem);
    if ((int16_t)rem >= 0) {                        /* 0x16359E -> 0x163520 */
        wr32(m, obj, 0x00163528u);
        return 0;
    }

    Obj_Free(m, obj);                               /* 0x1635A0..0x1635AC */
    return 0;
}
