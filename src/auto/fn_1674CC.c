/* ROM 0x1674CC: line-RAM palette fade-out ramp tick (A0 = object).
 * Computes fade level d0 = 8 - ((s16)(+0x44) >> 5) (clamped to <= 8) and
 * d1 = 4, packs inverted RGB444 word ~(((d1<<8)+(8-d1))<<4 | ((d0<<8)+(8-d0))),
 * fills 0xE8 words at 0x626200 + (s16)(A5-0x5D56), decrements +0x44, re-arming
 * 0x1674CC while >= 0 or freeing itself (0x0105EC) when < 0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1674CC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t d0w = (int16_t)(8 - ASR16(rd16(m, obj + 0x44u), 5)); /* 0x1674CC..0x1674D4 */
    int16_t d1w = 4;                                              /* 0x1674D6 */

    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    r->d[1] = setw(r->d[1], (uint16_t)d1w);

    uint16_t c1 = (uint16_t)d1w & 0x000Fu;                        /* 0x1674EC */
    if ((int16_t)c1 > 8)
        c1 = 8u;
    uint16_t c0 = (uint16_t)d0w & 0x000Fu;                        /* 0x1674FC */
    if ((int16_t)c0 > 8)
        c0 = 8u;

    uint16_t p1 = (uint16_t)(((c1 << 8) + (8u - c1)) << 4);       /* 0x16750C..0x167516 */
    uint16_t p0 = (uint16_t)((c0 << 8) + (8u - c0));              /* 0x167518..0x167520 */
    uint16_t fill = (uint16_t)~(p1 | p0);                         /* 0x167522..0x167524 */

    if (m->gfx) {
        uint32_t dst = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        for (int i = 0xE7; i >= 0; i--)                           /* 0x167526..0x167532 */
            wr16(m, dst + (uint32_t)(i * 2), fill);
    }

    int16_t cnt = (int16_t)(rd16(m, obj + 0x44u) - 1u);           /* 0x16753E */
    wr16(m, obj + 0x44u, (uint16_t)cnt);
    if (cnt >= 0) {                                               /* 0x167542 */
        wr32(m, obj, 0x001674CCu);                                /* 0x1674C4 */
        return 0;
    }

    Obj_Free(m, obj);                                             /* 0x167544..0x16754C */
    return 0;
}
