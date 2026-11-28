/* ROM 0x091466: draw a two-line debug text row and blank the rest of it
 * (A1 = record with a 0x0C-byte header, D1 = row, D2 = column pair).
 * 0x005666 draws the string at A1+0x0C at text RAM 0x61C000 + the 16-bit
 * offset built from 2*D2 + (D1<<7) (attribute 7); the returned cursor becomes
 * A1, then D2 advances by 0x0C and the ":" banner at 0x0913B8 is drawn the
 * same way. Finally words are cleared from the cursor on until one whose low
 * byte is that same ':' so the rest of the row is blank. */
#include "fn_auto.h"
#include "../enemy/obj_1194C2.h"     /* Text_DrawString (0x005666) */

#define TEXT_BASE 0x0061C000u
#define BANNER    0x000913B8u        /* ":" */

int Fn_091466(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1 = r->a[1] + 0x0Cu;                       /* 0x09146A lea (0xc,A1),A1 */
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];

    /* 0x091476..0x09147E: lsl.w #7,D1w ; add.w D2w,D2w ; add.w D1w,D2w ;
     * ext.l D2 ; addi.l #0x61C000,D2 */
    uint16_t w1 = (uint16_t)(d1 << 7);
    uint16_t w2 = (uint16_t)((uint16_t)(d2 << 1) + w1);
    uint32_t dst = (uint32_t)(int32_t)(int16_t)w2 + TEXT_BASE;
    uint32_t d0 = Text_DrawString(m, a1, dst, 7u);       /* 0x091488 */

    a1 = d0;                                             /* 0x091496 movea.l D0,A1 */
    d2 = (d2 & 0xFFFF0000u) | (uint16_t)((uint16_t)d2 + 0x000Cu); /* 0x091498 */

    /* 0x0914A0..0x0914B4: the same address computation with the new column. */
    w1 = (uint16_t)(d1 << 7);
    w2 = (uint16_t)((uint16_t)(d2 << 1) + w1);
    dst = (uint32_t)(int32_t)(int16_t)w2 + TEXT_BASE;
    d0 = Text_DrawString(m, BANNER, dst, 7u);            /* 0x0914BA */

    d1 = r->d[1];                                        /* 0x0914C4 movem restore */
    d2 = (r->d[2] & 0xFFFF0000u) | (uint16_t)((uint16_t)r->d[2] + 0x000Cu);

    for (;;) {                                           /* 0x0914C8 */
        d2 = (d2 & 0xFFFF0000u) | rd16(m, a1);           /* 0x0914C8 move.w (A1),D2w */
        if ((uint8_t)d2 == rd8(m, BANNER))               /* 0x0914CA cmp.b */
            break;
        wr16(m, a1, 0u);                                 /* 0x0914D2 clr.w (A1)+ */
        a1 += 2u;
    }

    r->a[1] = a1;
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
