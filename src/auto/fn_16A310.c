/* ROM 0x16A310: stage scroll/background init (A5-relative state).
 * Clears 22 longs at A5+0x4204..0x4258 and 8 longs at A5-0x5DA8..-0x5D8C,
 * selects entry from 0x16ABCE[A5+0x32DC] into A5+0x4288, loads initial scroll
 * deltas and counter via 0x16AB78, increments A5+0x4292, and clears A5+0x4294. */
#include "fn_auto.h"

/* ROM 0x16AB78: load scroll base/delta/counter from descriptor at *(A5+0x4288). */
static void sub_16AB78(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = rd32(m, F3_A5 + 0x4288u);                  /* 0x16AB78 */
    wr32(m, F3_A5 + 0x4224u, rd32(m, a0));                  /* 0x16AB7C */
    int32_t d0 = (int32_t)(int16_t)rd16(m, a0 + 4u) << 4;   /* 0x16AB80..0x16AB86 */
    wr32(m, F3_A5 + 0x4228u, (uint32_t)d0);                 /* 0x16AB88 */
    wr16(m, F3_A5 + 0x4292u, rd16(m, a0 + 6u));             /* 0x16AB8C */
    r->a[0] = a0;
    r->d[0] = (uint32_t)d0;
}

int Fn_16A310(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = F3_A5 + 0x4204u;                          /* 0x16A310 */
    for (int i = 0; i <= 0x15; i++) {                       /* 0x16A314..0x16A31A */
        wr32(m, a0, 0u);
        a0 += 4u;
    }

    a0 = F3_A5 - 0x5DA8u;                                   /* 0x16A31E */
    for (int i = 0; i <= 7; i++) {                          /* 0x16A322..0x16A328 */
        wr32(m, a0, 0u);
        a0 += 4u;
    }

    int16_t idx = (int16_t)((int16_t)rd16(m, F3_A5 + 0x32DCu) << 2); /* 0x16A332..0x16A336 */
    uint32_t ptr = rd32(m, 0x0016ABCEu + (uint32_t)(int32_t)idx); /* 0x16A32C, 0x16A338 */
    wr32(m, F3_A5 + 0x4288u, ptr);

    sub_16AB78(m, r);                                       /* 0x16A33E */
    wr16(m, F3_A5 + 0x4292u, (uint16_t)(rd16(m, F3_A5 + 0x4292u) + 1u)); /* 0x16A342 */
    wr16(m, F3_A5 + 0x4294u, 0u);                           /* 0x16A346 */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0x0000FFFFu;
    return 0;
}
