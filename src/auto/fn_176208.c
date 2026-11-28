/* ROM 0x176208: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_176208(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x176208: link.w A6, 0 */
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[6]);
    r->a[6] = r->a[7];

    /* ROM 0x17620C: movem.l { D7 D6 }, -(SP) */
    r->a[7] -= 8u;
    wr32(m, r->a[7], r->d[7]);
    wr32(m, r->a[7] + 4u, r->d[6]);

    /* ROM 0x176210: movea.l (0x8,A6), A0 */
    r->a[0] = rd32(m, r->a[6] + 8u);

    /* ROM 0x176214: adda.w (-0x5d56,A5), A0 */
    r->a[0] += (int16_t)rd16(m, F3_A5 - 0x5d56u);

    /* ROM 0x176218: move.w (0x10,A6), D7w */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | (uint16_t)rd16(m, r->a[6] + 16u);

    /* ROM 0x17621C: move.w (0xe,A6), D6w */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | (uint16_t)rd16(m, r->a[6] + 14u);

    /* ROM 0x176220: move.w (0x10,A6), D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)rd16(m, r->a[6] + 16u);

    /* ROM 0x176224: loop start */
    do {
        uint32_t addr = r->a[0] + (uint32_t)(int16_t)r->d[7] * 2u;
        wr16(m, addr, (uint16_t)r->d[0]);
        r->d[7] = (r->d[7] & 0xFFFF0000u) | ((uint16_t)r->d[7] + 1u);
    } while ((int16_t)r->d[7] <= (int16_t)r->d[6]);

    /* ROM 0x17622E: movem.l (SP)+, { D6 D7 } */
    r->d[7] = rd32(m, r->a[7]);
    r->d[6] = rd32(m, r->a[7] + 4u);
    r->a[7] += 8u;

    /* ROM 0x176232: unlk A6 */
    r->a[6] = rd32(m, r->a[6]);
    r->a[7] += 4u;

    /* ROM 0x176234: rts */
    return 0;
}
