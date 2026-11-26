/* ROM 0x15F95A: line-RAM brightness ramp tick (A0 = object).
 * Steps the 0x60-word strip at 0x62A400 + (int16_t)(A5-0x5D56) by +/- 0x0E
 * (XOR 0x3F before/after) depending on (A5-0x5D58) == 0 vs != 0, and
 * re-arms 0x15F95A via 0x15F952. */
#include "fn_auto.h"

int Fn_15F95A(const f3mem_t *m, f3regs_t *r)
{
    uint16_t d1w = 0x000Eu;                                  /* 0x15F95E */
    if (rd16(m, F3_A5 - 0x5D58u) != 0u)                      /* 0x15F962..0x15F966 */
        d1w = (uint16_t)(0u - d1w);                          /* 0x15F96A */

    uint32_t a1 = 0x0062A400u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x15F96C..0x15F972 */
    uint16_t d0w = rd16(m, a1);                              /* 0x15F976 */
    d0w ^= 0x003Fu;                                          /* 0x15F978 */
    d0w = (uint16_t)(d0w + d1w);                             /* 0x15F97C */
    d0w ^= 0x003Fu;                                          /* 0x15F97E */

    for (int16_t d7w = 0x005F; d7w >= 0; d7w--)              /* 0x15F982..0x15F98A */
        wr16(m, a1 + (uint32_t)(int32_t)d7w * 2u, d0w);

    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0xFFFFu;
    r->a[1] = a1;
    wr32(m, r->a[0], 0x0015F95Au);                           /* 0x15F992 -> 0x15F952 */
    return 0;
}
