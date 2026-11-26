/* ROM 0x176130: zero 0x1D0 bytes at (SP+4).l + (A5-0x5D56).w (line/gfx buffer). */
#include "fn_auto.h"

int Fn_176130(const f3mem_t *m, f3regs_t *r)
{
    uint32_t base = rd32(m, r->a[7] + 4u);                /* 0x176138 (0x8,A6) */
    int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u);      /* 0x17613C (-0x5D56,A5) */
    uint32_t dst = (uint32_t)((int32_t)base + (int32_t)off);

    for (uint32_t p = dst + 0x1D0u; p > dst; p -= 4u)     /* 0x176140..0x17618C */
        wr32(m, p - 4u, 0u);

    r->a[0] = dst;
    return 0;
}
