/* ROM 0x1784D8: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void Fn_00178502(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x178502: lea (0x42b4,A5),A0 */
    r->a[0] = F3_A5 + 0x42B4u;
    /* ROM 0x178506: lea (0x3ba,PC),A1 */
    r->a[1] = 0x1788C2u;
    /* ROM 0x17850A: nop */
    /* ROM 0x17850C: lea (0x3c8,PC),A2 */
    r->a[2] = 0x1788D6u;
    /* ROM 0x178510: nop */
    /* ROM 0x178512: move.w (0x4ab4,A5),D6w */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB4u);
    /* ROM 0x178516: move.w (0x4ab8,A5),D5w */
    r->d[5] = (r->d[5] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB8u);
    /* ROM 0x17851A: move.w (0x4ab6,A5),D4w */
    r->d[4] = (r->d[4] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB6u);
    /* ROM 0x17851E: move.w (0x0,A1,D4w*0x2),D3w */
    r->d[3] = (r->d[3] & 0xFFFF0000u) | rd16(m, r->a[1] + (uint32_t)(int16_t)r->d[4] * 2u);
    /* ROM 0x178522: add.w D5w,D6w */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | ((uint16_t)r->d[6] + (uint16_t)r->d[5]);
    /* ROM 0x178524: muls.w #0x8,D6 */
    r->d[6] = (int32_t)(int16_t)r->d[6] * 8;
    /* ROM 0x178528: lea (0x0,A0,D6w*0x1),A0 */
    r->a[0] += (uint32_t)(int16_t)r->d[6];
    /* ROM 0x17852C: cmpi.w #0x6,D4w */
    /* ROM 0x178530: bge.w 0x00178548 */
    if ((int16_t)r->d[4] >= 6) {
        /* ROM 0x178548: move.b (0x0,A0,D3w*0x1),D1b */
        uint8_t v1 = rd8(m, r->a[0] + (uint32_t)(int16_t)r->d[3]);
        /* ROM 0x17854C: cmp.b (0x0,A2,D3w*0x1),D1b */
        uint8_t v2 = rd8(m, r->a[2] + (uint32_t)(int16_t)r->d[3]);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | v1;
        /* ROM 0x178550: beq.w 0x00178558 */
        if (v1 == v2) {
            /* ROM 0x178558: rts */
            return;
        }
        /* ROM 0x178554: addq.b #0x1,(0x0,A0,D3w*0x1) */
        wr8(m, r->a[0] + (uint32_t)(int16_t)r->d[3], v1 + 1);
    } else {
        /* ROM 0x178534: move.w (0x0,A0,D3w*0x1),D1w */
        uint16_t v1 = rd16(m, r->a[0] + (uint32_t)(int16_t)r->d[3]);
        /* ROM 0x178538: cmp.w (0x0,A2,D3w*0x1),D1w */
        uint16_t v2 = rd16(m, r->a[2] + (uint32_t)(int16_t)r->d[3]);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | v1;
        /* ROM 0x17853C: beq.w 0x00178558 */
        if (v1 == v2) {
            /* ROM 0x178558: rts */
            return;
        }
        /* ROM 0x178540: addq.w #0x1,(0x0,A0,D3w*0x1) */
        wr16(m, r->a[0] + (uint32_t)(int16_t)r->d[3], v1 + 1);
    }
    /* ROM 0x178558: rts */
}

int Fn_1784D8(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1784D8: move.w #0xf,D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0xFu;
    /* ROM 0x1784DC: dbf D0w,0x001784dc */
    uint16_t d0_val = (uint16_t)r->d[0];
    do {
        /* ROM 0x1784DC: bsr.w 0x00178502 */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x1784E0u);
        Fn_00178502(m, r);
        r->a[7] += 4;
        d0_val--;
        r->d[0] = (r->d[0] & 0xFFFF0000u) | d0_val;
    } while ((int16_t)d0_val != -1);
    /* ROM 0x1784E4: rts */
    return 0;
}
