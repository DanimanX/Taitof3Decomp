/* ROM 0x167A04: stage-start palette fades for the background layers. Copies
 * three master palettes from ROM into the Work-RAM palette buffers
 * (0x23AC0 -> 0x40B42E, 32 colours; 0x23F40 -> 0x40B4AE, 16 colours;
 * 0x1BB80 -> 0x40B52E, 32 colours), then arms three 0x80-frame fades in the
 * first free palette-ramp slots (0x183C -> 0x1792): 0x167F2E->0x23AC0 (32
 * pixels, arg 0xF80), 0x167FAE->0x23F40 (16, 0x1400) and 0x1680AE->0x1BB80
 * (32, 0x7680). A0/A1/D0 are saved around each copy and D0 at the rts is the
 * slot address the last fade claimed (0xFFFFFFFF if no slot was free). */
#include "fn_auto.h"

/* ROM 0x0016A19C: move.l (A0)+,(A1)+ for D0.w+1 words. */
static void f167a04_copy_16A19C(const f3mem_t *m, uint32_t a0, uint32_t a1, uint16_t d0)
{
    uint32_t n = (uint32_t)d0;
    for (;;) {
        wr32(m, a1, rd32(m, a0));
        a0 += 4u;
        a1 += 4u;
        if (n == 0u)
            break;
        n--;                                                        /* 0x16A19E dbf */
    }
}

/* ROM 0x0017D6: per-colour {start.w,dstep.w} triples into A5-0x7F50 + slotoff,
 * fading A0's longs towards A1's longs over `frames` frames. */
static void f167a04_ramp_0017D6(const f3mem_t *m, uint32_t a0, uint32_t a1,
                                uint32_t frames, uint32_t count, uint32_t slotoff)
{
    uint32_t a2 = (F3_A5 - 0x7F50u) + slotoff;
    int32_t div = (int32_t)frames;
    uint16_t d7 = (uint16_t)(count - 1u);

    for (;;) {
        uint32_t c0 = rd32(m, a0);                                  /* 0x17E2 */
        uint32_t c1;
        a0 += 4u;
        c1 = rd32(m, a1);                                           /* 0x17E4 */
        a1 += 4u;
        for (uint32_t k = 0; k < 3u; k++) {
            unsigned sh = 16u - 8u * k;
            int32_t s = (int32_t)(((c0 >> sh) & 0xFFu) << 8);
            int32_t e, step;
            wr16(m, a2 + 2u * k, (uint16_t)s);                       /* 0x17EC */
            e = (int32_t)(((c1 >> sh) & 0xFFu) << 8);
            step = (e - s) / div;                                   /* 0x17F6 divsl.l */
            wr16(m, a2 + 6u + 2u * k, (uint16_t)step);               /* 0x17FA */
        }
        a2 += 0x0Cu;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
}

/* ROM 0x001792 (via 0x183C): claim the first free 6-byte slot of A5-0x6650
 * and build the ramp. Returns the slot address, or -1 when all 16 are busy. */
static uint32_t f167a04_claim_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                                     uint32_t d0, uint32_t frames, uint32_t count)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint16_t d6 = 0u;
    uint16_t d7 = 0x000Fu;

    for (;;) {
        if (rd16(m, a3) == 0u) {                                    /* 0x17A2 */
            wr16(m, a3 + 2u, (uint16_t)frames);                     /* 0x17A6 */
            if ((int16_t)(uint16_t)frames <= 0)                     /* 0x17AA */
                return a3;
            wr16(m, a3, (uint16_t)count);                           /* 0x17AC */
            if ((uint16_t)count == 0u)                              /* 0x17AE */
                return a3;
            wr16(m, a3 + 4u, (uint16_t)d0);                         /* 0x17B0 */
            f167a04_ramp_0017D6(m, a0, a1, frames, count, d6);       /* 0x17B4 */
            return a3;                                             /* 0x17B8 movea.l A3,A0 */
        }
        d6 = (uint16_t)(d6 + 0x0180u);                              /* 0x17BC */
        a3 += 6u;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            return 0xFFFFFFFFu;                                     /* 0x17C8 suba.l A0,A0 / -1 */
    }
}

int Fn_167A04(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0;

    /* 0x167A04..0x167A24: save {D0 A0 A1}, 32 longs 0x23AC0 -> 0x40B42E */
    f167a04_copy_16A19C(m, 0x00023AC0u, 0x0040B42Eu, (uint16_t)(0x0020u - 1u));
    /* 0x167A28..0x167A48: 16 longs 0x23F40 -> 0x40B4AE */
    f167a04_copy_16A19C(m, 0x00023F40u, 0x0040B4AEu, (uint16_t)(0x0010u - 1u));
    /* 0x167A4C..0x167A6C: 32 longs 0x1BB80 -> 0x40B52E */
    f167a04_copy_16A19C(m, 0x0001BB80u, 0x0040B52Eu, (uint16_t)(0x0020u - 1u));

    /* 0x167A70..0x167A94: push 0x20, 0x80, 0xF80, 0x23AC0, 0x167F2E -> 0x183C */
    (void)f167a04_claim_001792(m, 0x00167F2Eu, 0x00023AC0u, 0x00000F80u, 0x0080u, 0x0020u);
    /* 0x167A98..0x167ABC: push 0x10, 0x80, 0x1400, 0x23F40, 0x167FAE */
    (void)f167a04_claim_001792(m, 0x00167FAEu, 0x00023F40u, 0x00001400u, 0x0080u, 0x0010u);
    /* 0x167AC0..0x167AE4: push 0x20, 0x80, 0x7680, 0x1BB80, 0x1680AE. 0x183C
     * returns 0x1792's A0 in D0, and 0x167A04 never restores D0 afterwards. */
    d0 = f167a04_claim_001792(m, 0x001680AEu, 0x0001BB80u, 0x00007680u, 0x0080u, 0x0020u);

    r->d[0] = d0;                                                  /* 0x167AE8 rts */
    return 0;
}
