/* ROM 0x1784CA: 16x16 ramp-up of one palette component (A5+0x42B4 palette).
 * D7 = 0xF outer, 0x1784D8 inner (D0 = 0xF), each pass runs 0x178502:
 * A0 = A5+0x42B4 + (A5+0x4AB4 + A5+0x4AB8) * 8, A1 = word index table
 * 0x1788C2 (indexed by A5+0x4AB6), A2 = target word table 0x1788D6; D3 =
 * that index; when (A5+0x4AB6) < 6 a word, else a byte of the palette is
 * compared with A2 and incremented by one while it differs. Thus 256
 * increments of one entry, one step per call of this routine. */
#include "fn_auto.h"

/* ROM 0x178502: one increment step of the palette entry (A0/A1/A2, D1,D3-D6). */
static void sub_178502(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = F3_A5 + 0x42B4u;                            /* 0x178502 lea */
    r->a[1] = 0x001788C2u;                                    /* 0x178506 lea (0x3BA,PC) */
    r->a[2] = 0x001788D6u;                                    /* 0x17850C lea (0x3C8,PC) */
    r->d[6] = (r->d[6] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB4u); /* 0x178512 */
    r->d[5] = (r->d[5] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB8u); /* 0x178516 */
    r->d[4] = (r->d[4] & 0xFFFF0000u) | rd16(m, F3_A5 + 0x4AB6u); /* 0x17851A */
    r->d[3] = (r->d[3] & 0xFFFF0000u)
            | rd16(m, r->a[1] + (uint32_t)(int32_t)(int16_t)r->d[4] * 2u); /* 0x17851E */
    r->d[6] = (r->d[6] & 0xFFFF0000u)
            | (uint16_t)((uint16_t)r->d[6] + (uint16_t)r->d[5]);           /* 0x178522 */
    r->d[6] = (uint32_t)((int32_t)(int16_t)r->d[6] * 8);                   /* 0x178524 muls.w #8 */
    r->a[0] = a0 + (uint32_t)(int32_t)(int16_t)r->d[6];                             /* 0x178528 lea D6w */

    uint32_t addr = r->a[0] + (uint32_t)(int32_t)(int16_t)r->d[3];               /* D3w, scale 1 */
    if ((int16_t)r->d[4] >= 6) {                                           /* 0x17852C/0x178530 */
        uint8_t v = rd8(m, addr);                                          /* 0x178548 */
        uint8_t tgt = rd8(m, r->a[2] + (uint32_t)(int32_t)(int16_t)r->d[3]); /* 0x17854C */
        r->d[1] = (r->d[1] & 0xFFFFFF00u) | v;
        if (v == tgt)                                                      /* 0x178550 */
            return;
        wr8(m, addr, (uint8_t)(v + 1u));                                   /* 0x178554 */
    } else {
        uint16_t v = rd16(m, addr);                                        /* 0x178534 */
        uint16_t tgt = rd16(m, r->a[2] + (uint32_t)(int32_t)(int16_t)r->d[3]); /* 0x178538 */
        r->d[1] = (r->d[1] & 0xFFFF0000u) | v;
        if (v == tgt)                                                      /* 0x17853C */
            return;
        wr16(m, addr, (uint16_t)(v + 1u));                                 /* 0x178540 */
    }
}

int Fn_1784CA(const f3mem_t *m, f3regs_t *r)
{
    /* 0x1784CA move.w #0xf,D7w */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0x000Fu;
    uint16_t d7 = 0x000Fu;
    do {                                                                   /* 0x1784CE */
        uint16_t d0 = 0x000Fu;                                             /* 0x1784D8 */
        do {                                                               /* 0x1784DC */
            sub_178502(m, r);                                              /* 0x1784DC bsr */
            d0--;                                                          /* 0x1784E0 dbf */
            r->d[0] = (r->d[0] & 0xFFFF0000u) | d0;
        } while (d0 != 0xFFFFu);
        d7--;                                                              /* 0x1784D2 dbf */
        r->d[7] = (r->d[7] & 0xFFFF0000u) | d7;
    } while (d7 != 0xFFFFu);
    return 0;                                                              /* 0x1784D6 rts */
}
