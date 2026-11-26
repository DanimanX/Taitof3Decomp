/* ROM 0x138470: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_138470(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x138470: movea.l (0xc,A0),A1 */
    r->a[1] = rd32(m, r->a[0] + 0xCu);
    /* 138474 move.w (A1)+,(0x22,A0) */
    wr16(m, r->a[0] + 0x22u, rd16(m, r->a[1]));
    r->a[1] += 2u;
    /* 138478 move.w (A1)+,(0x50,A0) */
    wr16(m, r->a[0] + 0x50u, rd16(m, r->a[1]));
    r->a[1] += 2u;
    /* 13847C move.w (A1)+,(0x52,A0) */
    wr16(m, r->a[0] + 0x52u, rd16(m, r->a[1]));
    r->a[1] += 2u;
    /* 138480 move.w (A1)+,(0x42,A0) */
    wr16(m, r->a[0] + 0x42u, rd16(m, r->a[1]));
    r->a[1] += 2u;
    /* 138484 move.l A1,(0xc,A0) */
    wr32(m, r->a[0] + 0xCu, r->a[1]);

    /* 138488 move.w (0x22,A0),D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)rd16(m, r->a[0] + 0x22u);
    /* 13848C lsl.w #0x4,D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | ((uint16_t)r->d[0] << 4);
    /* 13848E lea (0x152,PC),A1 */
    r->a[1] = 0x138490u + 0x152u;
    /* 138492 nop */
    /* 138494 movea.l (0x0,A1,D0w*0x1),A1 */
    r->a[1] = rd32(m, r->a[1] + (int32_t)(int16_t)r->d[0]);
    /* 138498 move.w (A1),(0x6,A0) */
    wr16(m, r->a[0] + 0x6u, rd16(m, r->a[1]));
    /* 13849C move.l (0x30d2,A5),(0x14,A0) */
    wr32(m, r->a[0] + 0x14u, rd32(m, F3_A5 + 0x30d2u));
    /* 1384A2 move.l (0x30d6,A5),(0x18,A0) */
    wr32(m, r->a[0] + 0x18u, rd32(m, F3_A5 + 0x30d6u));
    /* 1384A8 move.l #0x1384b0,(A0) */
    wr32(m, r->a[0], 0x1384B0u);
    /* 1384AE rts */
    return 0;
}
