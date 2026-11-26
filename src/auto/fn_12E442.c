/* ROM 0x12E442: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_12E442(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x12E442: Load D0.w from A0+0x42, add to A5+0x32A2 */
    uint16_t d0 = (uint16_t)rd16(m, r->a[0] + 0x42);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0;
    int16_t a5_val = (int16_t)(uint16_t)rd16(m, 0x408000u + 0x32a2);
    wr16(m, 0x408000u + 0x32a2, (uint16_t)(a5_val + (int16_t)d0));

    /* ROM 0x12E44A: Load D0.w from A5+0x32A2, subtract A0+0x20, abs */
    d0 = (uint16_t)rd16(m, 0x408000u + 0x32a2);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0;
    int16_t sub_val = (int16_t)(uint16_t)rd16(m, r->a[0] + 0x20);
    int16_t res = (int16_t)d0 - sub_val;
    if (res < 0) res = -res;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)res;

    /* ROM 0x12E45A: Compare D0.w with A0+0x22 */
    uint16_t limit = (uint16_t)rd16(m, r->a[0] + 0x22);
    if ((uint16_t)r->d[0] >= limit) {
        /* ROM 0x12E46C: Re-arm routine at 0x12E43A */
        wr32(m, r->a[0], 0x12E442);
        return 0;
    }

    /* ROM 0x12E462: Update A5+0x32A2 from A0+0x20 */
    wr16(m, 0x408000u + 0x32a2, (uint16_t)rd16(m, r->a[0] + 0x20));

    /* ROM 0x12E46E: Jump to A1 */
    r->a[1] = rd32(m, r->a[0] + 0xC);
    /* jmp (A1) - control leaves here */
    return 0;
}
