/* ROM 0x1784F4: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void Fn_17855A(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x17855A: lea (0x42b4, A5), A0 */
    r->a[0] = F3_A5 + 0x42b4u;
    /* ROM 0x17855E: lea (0x362, PC), A1 */
    r->a[1] = 0x178560u + 0x362u;
    /* ROM 0x178562: nop */
    /* ROM 0x178564: lea (0x378, PC), A2 */
    r->a[2] = 0x178566u + 0x378u;
    /* ROM 0x178568: nop */
    /* ROM 0x17856A: move.w (0x4ab4, A5), D6.w */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4ab4u);
    /* ROM 0x17856E: move.w (0x4ab8, A5), D5.w */
    r->d[5] = (r->d[5] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4ab8u);
    /* ROM 0x178572: move.w (0x4ab6, A5), D4.w */
    r->d[4] = (r->d[4] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4ab6u);
    /* ROM 0x178576: move.w (0x0, A1, D4.w*0x2), D3.w */
    r->d[3] = (r->d[3] & 0xFFFF0000u) | rd16(m, r->a[1] + (uint32_t)(int16_t)r->d[4] * 2u);
    /* ROM 0x17857A: add.w D5.w, D6.w */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | ((uint16_t)r->d[6] + (uint16_t)r->d[5]);
    /* ROM 0x17857C: muls.w #0x8, D6 */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | ((int16_t)(uint16_t)r->d[6] * 8);
    /* ROM 0x178580: lea (0x0, A0, D6.w*0x1), A0 */
    r->a[0] += (int32_t)(int16_t)r->d[6];
    /* ROM 0x178584: cmpi.w #0x6, D4.w */
    /* ROM 0x178588: bge.w 0x001785a0 */
    if ((int16_t)(uint16_t)r->d[4] >= 6) {
        /* ROM 0x1785A0: move.b (0x0, A0, D3.w*0x1), D1.b */
        r->d[1] = (r->d[1] & 0xFF000000u) | rd8(m, r->a[0] + (uint32_t)(int16_t)r->d[3]);
        /* ROM 0x1785A4: cmp.b (0x0, A2, D3.w*0x1), D1.b */
        /* ROM 0x1785A8: beq.w 0x001785b0 */
        if ((uint8_t)rd8(m, r->a[2] + (uint32_t)(int16_t)r->d[3]) == (uint8_t)r->d[1]) {
            return;
        }
        /* ROM 0x1785AC: subq.b #0x1, (0x0, A0, D3.w*0x1) */
        wr8(m, r->a[0] + (uint32_t)(int16_t)r->d[3], (uint8_t)rd8(m, r->a[0] + (uint32_t)(int16_t)r->d[3]) - 1);
    } else {
        /* ROM 0x17858C: move.w (0x0, A0, D3.w*0x1), D1.w */
        r->d[1] = (r->d[1] & 0xFFFF0000u) | rd16(m, r->a[0] + (uint32_t)(int16_t)r->d[3]);
        /* ROM 0x178590: cmp.w (0x0, A2, D3.w*0x1), D1.w */
        /* ROM 0x178594: beq.w 0x001785b0 */
        if (rd16(m, r->a[2] + (uint32_t)(int16_t)r->d[3]) == (uint16_t)r->d[1]) {
            return;
        }
        /* ROM 0x178598: subq.w #0x1, (0x0, A0, D3.w*0x1) */
        wr16(m, r->a[0] + (uint32_t)(int16_t)r->d[3], (uint16_t)rd16(m, r->a[0] + (uint32_t)(int16_t)r->d[3]) - 1);
    }
    /* ROM 0x1785B0: rts */
}

int Fn_1784F4(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1784F4: move.w #0xf, D0.w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x000Fu;
    /* ROM 0x1784F8: bsr.w 0x0017855a */
    /* ROM 0x1784FC: dbf D0.w, 0x001784f8 */
    for (;;) {
        Fn_17855A(m, r);
        uint16_t d0w = (uint16_t)r->d[0] - 1u;
        r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
        if (d0w == 0xFFFFu) break;
    }
    /* ROM 0x178500: rts */
    return 0;
}
