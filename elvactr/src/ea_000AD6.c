/* ROM 0x000AD6: compute a status/flag value in D0 based on A5 offsets */
#include "ea_auto.h"

int Ea_000AD6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t d0 = r->d[0];
    uint16_t v_a2 = rd16(m, a5 - 0x7F5Eu);
    uint16_t v_90 = rd16(m, a5 - 0x7F70u);
    uint16_t v_92 = rd16(m, a5 - 0x7F6Eu);
    uint16_t v_a4 = rd16(m, a5 - 0x7F5Cu);

    /* 0x000AD6 tst.w -$7f5e(a5) */
    if (v_a2 == 0) {
        /* 0x000AF8 tst.w -$7f70(a5) */
        /* 0x000AFC seq.b d0 */
        if (v_90 == 0)
            d0 = (d0 & 0xFFFFFF00u) | 0xFFu;
        else
            d0 &= 0xFFFFFF00u;
    } else {
        /* 0x000ADC tst.w -$7f70(a5) */
        if (v_90 != 0) {
            /* 0x000AF4 moveq #$0, d0 */
            d0 = 0u;
        } else {
            /* 0x000AE2 move.w -$7f6e(a5), d0 */
            d0 = v_92;
            /* 0x000AE6 bne.b $aec */
            if (d0 != 0) {
                /* 0x000AEC neg.w d0 */
                d0 = (uint16_t)(-(int16_t)d0);
                /* 0x000AEE add.w -$7f5c(a5), d0 */
                d0 = (uint16_t)((int16_t)d0 + (int16_t)v_a4);
            } else {
                /* 0x000AE8 moveq #$ff, d0 */
                d0 = 0xFFu;
            }
        }
    }

    r->d[0] = d0;
    r->a[7] = r->a[7] + 4u;
    return 0;
}
