/* ROM 0x11CF34: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../kernel/rng.h"

static void FUN_000cbeb0(const f3mem_t *m, f3regs_t *r) {
    int32_t d1 = (int32_t)(int16_t)((rd16(m, F3_A5 + 0x87e) << 3) + 0xc00);
    int32_t d0 = (int32_t)(int16_t)r->d[0] * d1;
    d0 <<= 4;
    d0 = (d0 << 16) | (d0 >> 16);
    r->d[0] = d0;
}

static void FUN_11CE2A(const f3mem_t *m, f3regs_t *r) {
    uint8_t v = rd8(m, F3_A5 + 0x30f2u);
    v |= 0x02u;
    wr8(m, F3_A5 + 0x30f2u, v);

    uint8_t v2 = rd8(m, F3_A5 + 0x30eeu);
    v2 |= (uint8_t)(1u << (7 - 6));
    wr8(m, F3_A5 + 0x30eeu, v2);
    v2 &= (uint8_t)~(1u << (7 - 5));
    wr8(m, F3_A5 + 0x30eeu, v2);

    wr16(m, r->a[0] + 6, 0x12cu);

    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)Rng_Next();
    r->d[0] = (r->d[0] & 0xFFFF0000u) | ((uint16_t)r->d[0] & 0x1fu);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | ((uint16_t)r->d[0] + 0xdcu);
    wr16(m, r->a[0] + 0x3au, (uint16_t)r->d[0]);

    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)Rng_Next();
    r->d[0] = (r->d[0] & 0xFFFF0000u) | ((uint16_t)r->d[0] & 0x7fu);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | ((uint16_t)r->d[0] + 0x40u);
    wr16(m, r->a[0] + 0x3cu, (uint16_t)r->d[0]);

    wr32(m, r->a[0], 0x11ce6eu);
}

int Fn_11CF34(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11CF34: OBJ_11CF34 */
    uint16_t cnt = (uint16_t)(rd16(m, r->a[0] + 6) - 1);
    wr16(m, r->a[0] + 6, cnt);
    if ((int16_t)cnt > 0) {
        /* 11CF2C: move.l #0x11cf34,(A0) ; rts */
        wr32(m, r->a[0], 0x11cf34u);
        return 0;
    }

    /* 11CF3A: bclr.b #0x1,(0x30f2,A5) */
    uint8_t v30f2 = rd8(m, F3_A5 + 0x30f2u);
    v30f2 &= (uint8_t)~0x02u;
    wr8(m, F3_A5 + 0x30f2u, v30f2);

    /* 11CF40: bfset (0x30ee,A5){0x7:0x1} */
    uint8_t v30ee = rd8(m, F3_A5 + 0x30eeu);
    v30ee |= (uint8_t)(1u << (7 - 7));
    wr8(m, F3_A5 + 0x30eeu, v30ee);

    /* 11CF46: move.w #0x3c,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x3cu);

    if (cnt == 0) {
        /* 11CF5C: bfclr (0x30ee,A5){0x3:0x1} */
        v30ee = rd8(m, F3_A5 + 0x30eeu);
        v30ee &= (uint8_t)~(1u << (7 - 3));
        wr8(m, F3_A5 + 0x30eeu, v30ee);

        /* 11CF62: move.w #0x64,D0w */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x64u;

        /* 11CF66: jsr 0x000cbeb0.l */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x11cf6cu);
        FUN_000cbeb0(m, r);
        r->a[7] += 4;

        /* 11CF6C: neg.w D0w */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)(-(int16_t)(r->d[0] & 0xFFFFu));

        /* 11CF6E: addi.w #0xa0,D0w */
        r->d[0] = (r->d[0] & 0xFFFF0000u) | ((uint16_t)(r->d[0] & 0xFFFFu) + 0xa0u);

        /* 11CF72: move.w D0w,(0x6,A0) */
        wr16(m, r->a[0] + 6, (uint16_t)r->d[0]);

        if ((int16_t)(r->d[0] & 0xFFFFu) == 0) {
            /* 11CF86: bfclr (0x30ee,A5){0x1:0x1} */
            v30ee = rd8(m, F3_A5 + 0x30eeu);
            v30ee &= (uint8_t)~(1u << (7 - 1));
            wr8(m, F3_A5 + 0x30eeu, v30ee);

            /* 11CF8C: bra.w 0x0011ce2a */
            FUN_11CE2A(m, r);
            return 0;
        }

        /* 11CF78: move.l #0x11cf80,(A0) ; rts */
        wr32(m, r->a[0], 0x11cf80u);
        return 0;
    }

    /* 11CF4E: move.l #0x11cf56,(A0) ; rts */
    wr32(m, r->a[0], 0x11cf56u);
    return 0;
}
