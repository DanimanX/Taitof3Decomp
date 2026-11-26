/* ROM 0x1379CC: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

int Fn_1379CC(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1379CC: cmpi.w #0x4, (0x32d6,A5) */
    uint16_t v = rd16(m, F3_A5 + 0x32D6u);
    if ((int16_t)v >= 4) {
        /* ROM 0x137B2C: clr.w (0x3318,A5) */
        wr16(m, F3_A5 + 0x3318u, 0u);
        /* ROM 0x137B30: move.b #0x2, (0x54,A0) */
        wr8(m, r->a[0] + 0x54u, 2u);
        /* ROM 0x137B36: move.w #0x1d0, (0x50,A0) */
        wr16(m, r->a[0] + 0x50u, 0x1D0u);
        /* ROM 0x137B3C: move.w #0x78, (0x52,A0) */
        wr16(m, r->a[0] + 0x52u, 0x78u);
        /* ROM 0x137B42: move.w #0x12c, (0x6,A0) */
        wr16(m, r->a[0] + 0x6u, 0x12Cu);
        /* ROM 0x137B48: beq.b 0x137B6A (never taken, 0x12C != 0) */
        /* ROM 0x137B4A: move.l #0x137B52, (A0) */
        wr32(m, r->a[0], 0x00137B52u);
        return 0;
    }

    /* ROM 0x1379D6: movea.l (0x30e2,A5),A1 */
    r->a[1] = rd32(m, F3_A5 + 0x30E2u);
    /* ROM 0x1379DA: move.w #0x195,D0 */
    r->d[0] = setw(r->d[0], 0x195u);
    /* ROM 0x1379DE: add.w (0x14,A1),D0 */
    r->d[0] = setw(r->d[0], (uint16_t)(r->d[0] + rd16(m, r->a[1] + 0x14u)));
    /* ROM 0x1379E2: sub.w (0x30d2,A5),D0 */
    r->d[0] = setw(r->d[0], (uint16_t)(r->d[0] - rd16(m, F3_A5 + 0x30D2u)));
    /* ROM 0x1379E6: sub.w (0x32ce,A5),D0 */
    r->d[0] = setw(r->d[0], (uint16_t)(r->d[0] - rd16(m, F3_A5 + 0x32CEu)));
    /* ROM 0x1379EA: tst.w D0 */
    /* ROM 0x1379EC: bgt.w 0x137A04 */
    if ((int16_t)r->d[0] > 0) {
        /* ROM 0x137A04: cmpi.w #0x200, (0x42,A0) */
        uint16_t v2 = rd16(m, r->a[0] + 0x42u);
        if ((int16_t)v2 >= 0x200) {
            /* branch to 0x137A14 */
        } else {
            /* ROM 0x137A0E: addi.w #0x40, (0x42,A0) */
            wr16(m, r->a[0] + 0x42u, (uint16_t)(v2 + 0x40u));
        }
        goto common_137A14;
    }

    /* ROM 0x1379F0: cmpi.w #-0x200, (0x42,A0) */
    uint16_t v4 = rd16(m, r->a[0] + 0x42u);
    if ((int16_t)v4 <= -0x200) {
        /* branch to 0x137A14 */
    } else {
        /* ROM 0x1379FA: subi.w #0x40, (0x42,A0) */
        wr16(m, r->a[0] + 0x42u, (uint16_t)(v4 - 0x40u));
    }
    /* ROM 0x137A00: bra.w 0x137A14 */

common_137A14:
    /* ROM 0x137A14: bfexts (0x42,A0){0:8},D0 */
    r->d[0] = (uint32_t)(int8_t)rd8(m, r->a[0] + 0x42u);
    /* ROM 0x137A1A: add.w D0, (0x32ce,A5) */
    wr16(m, F3_A5 + 0x32CEu, (uint16_t)(rd16(m, F3_A5 + 0x32CEu) + (int16_t)r->d[0]));
    /* ROM 0x137A1E: add.w D0, (0x32d2,A5) */
    wr16(m, F3_A5 + 0x32D2u, (uint16_t)(rd16(m, F3_A5 + 0x32D2u) + (int16_t)r->d[0]));
    /* ROM 0x137A22: subq.w #1, (0x6,A0) */
    uint16_t v3 = rd16(m, r->a[0] + 0x6u);
    v3 = (uint16_t)(v3 - 1u);
    wr16(m, r->a[0] + 0x6u, v3);
    /* ROM 0x137A26: bgt.b 0x1379C4 */
    if ((int16_t)v3 > 0) {
        wr32(m, r->a[0], 0x001379CCu);
        return 0;
    }
    /* ROM 0x137A28: move.w #0x1e, (0x6,A0) */
    wr16(m, r->a[0] + 0x6u, 0x1Eu);
    /* ROM 0x137A2E: beq.b 0x137A3E (never taken) */
    /* ROM 0x137A30: move.l #0x137A38, (A0) */
    wr32(m, r->a[0], 0x00137A38u);
    return 0;
}
