/* ROM 0x0C2DF2: screen/line-RAM reset for the 0x0C2Exx stage script (no
 * arguments; A0 is saved across the whole body).
 * 0x16AB2C writes {0x00080000, 0} to the camera mirror A5+0x4204 and to
 * A5-0x5DA8, 0x16AB26 clears A5+0x4224, 0x176198 fills the 0x1D0-byte band at
 * 0x628000 + A5-0x5D56 with 0x0080, 0x176130 zeroes the band at 0x62A000 +
 * the same offset, and Pal_FadeFromBlack(0x201C0, 0x700, 0x1E, 0x20) queues
 * the ramp (0x001866 -> 0x001792) and returns its slot in D0 (kept). The tick
 * is then parked at 0x0C2E80. */
#include "fn_auto.h"
#include "../objects/obj_common.h"      /* Mem_WritePair64 (0x16AB2C) */
#include "../objects/obj_shared_ext.h"   /* Obj_16AB26 */
#include "../video/palette.h"            /* Pal_QueueRamp (0x001792) */

static void fill_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0u; off < 0x01D0u; off += 2u)   /* 0x1761C4..0x1761FC */
        wr16(m, a0 + off, val);
}

int Fn_0C2DF2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];                             /* 0x0C2DF2 movem.l {A0} */
    uint32_t a1 = F3_A5 - 0x5DA8u;                      /* 0x0C2DFA */

    Mem_WritePair64(m, F3_A5 + 0x4204u, a1, 0x00080000u, 0u); /* 0x0C2DFE..0x0C2E0E */
    a1 += 4u;                                           /* 0x16AB2C left A1 = A1+4 */
    Obj_16AB26(m, F3_A5 + 0x4224u, 0u, 0u);             /* 0x0C2E14..0x0C2E28 */
    fill_176198(m, 0x00628000u, 0x0080u);               /* 0x0C2E2E..0x0C2E3E */

    r->a[7] -= 4u;                                      /* 0x0C2E42 pea 0x62A000 */
    wr32(m, r->a[7], 0x0062A000u);
    (void)Fn_176130(m, r);                              /* 0x0C2E48 */
    r->a[7] += 4u;                                      /* 0x0C2E4E lea (4,SP),SP */

    /* 0x0C2E52..0x0C2E70: Pal_FadeFromBlack(0x201C0, 0x700, 0x1E, 0x20); its
     * D0 is the ramp slot 0x001866 returned from 0x001792. */
    uint32_t d0 = Pal_QueueRamp(m, 0x0000190Eu, 0x000201C0u, 0x00000700u,
                                0x001Eu, 0x0020u);
    if (d0 == 0u)                                       /* 0x0017C8 suba.l/subq.l */
        d0 = 0xFFFFFFFFu;                                /* no free ramp slot */

    wr32(m, obj + 0x00u, 0x000C2E80u);                  /* 0x0C2E78 */
    r->d[0] = d0;
    r->d[1] = 0u;                                       /* 0x0C2E24 move.l (SP)+,D1 */
    r->a[0] = obj;
    r->a[1] = a1;
    return 0;
}
