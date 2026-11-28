/* ROM 0x0B7FF8: (recipe capture fnk_, work/zcap/recipes15.tsv): toggle the playfield
 * row-interleave and rotate every row.
 * The capture work/ram/fn_0B7FF8 has every post snapshot one frame later than
 * its pre (0x6DD -> 0x6DE, ...), so the harness skips all 30 pairs as
 * "interrupted by VBlank" and the answer cannot be compared against it.
 * The flag word A5-0x5D58 is inverted; when it becomes 0 the row offset
 * A5-0x5D56 takes 0x30, otherwise 0. For each of 8 rows of 4 0x200-byte
 * sub-rows the routine copies 256 words out of 0x624000 at the old offset into
 * the 0x624000 scratch row and copies them back at the new offset (D5 wraps
 * with `andi.w #0x1FE`), then patches the 0x626000/0x6261FE line-RAM word and
 * the matching 0x2E/0x1D0 slot, and finally calls 0x001070 (sprite buffers) and
 * 0x001C2C (scroll base table). The whole register file is restored. */
#include "fn_auto.h"
#include "../video/sprite.h"          /* Sprite_InitBuffers (0x001070) */

#define ROW_OFF   (F3_A5 - 0x5D56u)
#define FLIP_FLAG (F3_A5 - 0x5D58u)
#define ROW_SCRATCH 0x00624000u

/* 0x0B8022..0x0B8038: rotate one 0x1FE-byte row out of a0s into the scratch
 * row (256 words, the source index wrapping at 0x1FE). */
static void row_out(const f3mem_t *m, uint32_t a0s, uint16_t off)
{
    uint32_t p = ROW_SCRATCH;
    uint16_t d5 = off;
    for (int i = 0; i < 0x100; i++) {                   /* dbf D4w, D4 = 0xFF */
        wr16(m, p, rd16(m, a0s + d5));                  /* 0x0B802E */
        p += 2u;
        d5 = (uint16_t)((d5 + 2u) & 0x01FEu);           /* 0x0B8032..0x0B8034 */
    }
}

/* 0x0B803C..0x0B8052: rotate the scratch row back out at the new offset. */
static void row_in(const f3mem_t *m, uint32_t a0s, uint16_t off)
{
    uint32_t p = ROW_SCRATCH;
    uint16_t d5 = off;
    for (int i = 0; i < 0x100; i++) {                   /* dbf D4w, D4 = 0xFF */
        wr16(m, a0s + d5, rd16(m, p));                  /* 0x0B8048 */
        p += 2u;
        d5 = (uint16_t)((d5 + 2u) & 0x01FEu);           /* 0x0B804C..0x0B804E */
    }
}

int Fn_0B7FF8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t save_a0 = r->a[0];
    uint16_t d0 = (uint16_t)rd16(m, ROW_OFF);           /* 0x0B7FFC */
    uint16_t flag = (uint16_t)~rd16(m, FLIP_FLAG);      /* 0x0B8000 not.w */
    wr16(m, FLIP_FLAG, flag);                           /* 0x0B8000 */
    /* 0x0B8004..0x0B8010: seq D1b (0xFF when the new flag is 0) / ext.w / andi */
    uint16_t d1 = (flag == 0u) ? 0x0030u : 0x0000u;
    wr16(m, ROW_OFF, d1);

    for (int row = 0; row < 8; row++) {                 /* 0x0B801A dbf D2w (D2 = 7) */
        uint32_t a0 = ROW_SCRATCH + (uint32_t)row * 0x1000u;
        for (int sub = 0; sub < 4; sub++) {             /* 0x0B801E dbf D3w (D3 = 3) */
            uint32_t a0s = a0 + (uint32_t)sub * 0x200u;
            row_out(m, a0s, d0);                        /* 0x0B8022..0x0B8038 */
            row_in(m, a0s, d1);                         /* 0x0B803C..0x0B8052 */
        }
    }

    if (rd16(m, FLIP_FLAG) == 0u) {                     /* 0x0B8066 tst/bne */
        wr16(m, 0x00626000u, 0x0800u);                  /* 0x0B806E */
        wr16(m, 0x0062602Eu, 0x0000u);                  /* 0x0B8076 */
    } else {
        wr16(m, 0x006261FEu, 0x0800u);                  /* 0x0B8082 */
        wr16(m, 0x006261D0u, 0x0000u);                  /* 0x0B808A */
    }

    Sprite_InitBuffers(m);                              /* 0x0B8092 -> 0x001070 */
    (void)Fn_001C2C(m, r);                              /* 0x0B8098 -> 0x001C2C */
    r->a[0] = save_a0;                                  /* 0x0B809E movem restore */
    return 0;
}
