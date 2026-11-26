/* ROM 0x1659F0: line-RAM gradient fill tick using 0x176B68 and table 0x16649E (A0 = object). */
#include "fn_auto.h"

int Fn_1659F0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    f3regs_t sub_r = *r;                                             /* 0x1659F0 movem A0 */
    sub_r.a[0] = 0x0062A600u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x1659F4..0x1659FA */
    sub_r.a[1] = 0x0016649Eu;                                        /* 0x1659FE */
    sub_r.d[7] &= 0xFFFF0000u;                                       /* 0x165A04 move.w #0,D7 */
    Fn_176B68(m, &sub_r);                                            /* 0x165A08 */

    for (int i = 0; i < 8; i++)
        r->d[i] = sub_r.d[i];
    r->a[1] = sub_r.a[1];                                            /* 0x165A0E restores A0 only */

    wr32(m, obj, 0x00165A1Au);                                       /* 0x165A12 */
    return 0;
}
