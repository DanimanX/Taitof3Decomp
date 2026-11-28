#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../video/sprite.h"
#include "../enemy/obj_1194C2.h"

/* ROM 0x178700: debug table dump for the test build (no object argument).
 *
 * Blank 28 rows of 24 cells from ROM 0x1788E6 over the text layer starting at
 * 0x61C002, every row coloured with the entry D0w, then print 26 rows of the
 * 8-byte records at A5+0x42B4 + (row + A5+0x4AB4)*8: three words as 4 hex
 * digits at columns 1/6/0xB and two bytes as 2 hex digits at columns 0x10/0x13
 * of the same row, all with attribute 7 (0x005690). The two headers ROM
 * 0x178916 / 0x1788FF go to 0x61C002 / 0x61C082 with attribute 9; D0 returns
 * the end of the last header. */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* 0x61C000 + 2*col + row*0x80, with the ROM's word arithmetic. */
static uint32_t cell_dst(uint32_t col, uint32_t row)
{
    uint16_t c = (uint16_t)col;
    uint16_t d6 = (uint16_t)((uint16_t)(c + c) + (uint16_t)((uint16_t)row << 7));
    return 0x0061C000u + (uint32_t)(int32_t)(int16_t)d6;
}

int Fn_178700(const f3mem_t *m, f3regs_t *r)
{
    /* 0x178700: clear the 28 x 24 cell block. D0 is not saved around the
     * call, so after the first row the colour is the low half of the previous
     * return address. */
    for (int32_t d7 = 0x1B; ; d7--) {                             /* 0x178732 dbf */
        uint16_t dstw = (uint16_t)(2u + (uint16_t)((uint16_t)d7 << 7)); /* 0x17870E..0x178710 */
        uint32_t dst = 0x0061C000u + (uint32_t)(int32_t)(int16_t)dstw; /* 0x178714..0x178716 */
        r->d[0] = Text_DrawString(m, 0x001788E6u, dst, (uint16_t)r->d[0]); /* 0x178724 */
        if (d7 == 0)
            break;
    }

    uint32_t a0 = F3_A5 + 0x42B4u;                                /* 0x178736 */

    for (int32_t d7 = 0x19; ; d7--) {                             /* 0x178850 dbf */
        uint16_t d5 = (uint16_t)(d7 + 2);                         /* 0x17873E */
        uint16_t d4w = (uint16_t)(d7 + rd16(m, F3_A5 + 0x4AB4u));  /* 0x178742..0x178744 */
        int32_t d4 = (int32_t)(int16_t)d4w * 8;                   /* 0x178748 muls.w #8 */
        uint32_t rowbase = a0 + (uint32_t)(int32_t)(int16_t)(uint16_t)d4; /* d4.w */

        /* The five fields of the record, in the ROM's order (D3 keeps the
         * upper bytes of the previous word on the two byte reads). */
        r->d[3] = setw(r->d[3], rd16(m, rowbase));                /* 0x178750 */
        (void)Gfx_DrawHex(m, r->d[3], 4u, cell_dst(1u, d5), 7u);  /* 0x178772 */
        r->d[3] = setw(r->d[3], rd16(m, rowbase + 2u));           /* 0x178784 */
        (void)Gfx_DrawHex(m, r->d[3], 4u, cell_dst(6u, d5), 7u);  /* 0x1787A6 */
        r->d[3] = setw(r->d[3], rd16(m, rowbase + 4u));           /* 0x1787B8 */
        (void)Gfx_DrawHex(m, r->d[3], 4u, cell_dst(0x0Bu, d5), 7u); /* 0x1787DA */
        r->d[3] = (r->d[3] & 0xFFFFFF00u) | rd8(m, rowbase + 6u); /* 0x1787EC */
        (void)Gfx_DrawHex(m, r->d[3], 2u, cell_dst(0x10u, d5), 7u); /* 0x17880E */
        r->d[3] = (r->d[3] & 0xFFFFFF00u) | rd8(m, rowbase + 7u); /* 0x178820 */
        (void)Gfx_DrawHex(m, r->d[3], 2u, cell_dst(0x13u, d5), 7u); /* 0x178842 */
        if (d7 == 0)
            break;
    }

    Text_DrawString(m, 0x00178916u, 0x0061C002u, 9u);             /* 0x178864 */
    r->d[0] = Text_DrawString(m, 0x001788FFu, 0x0061C082u, 9u);  /* 0x17887E */
    r->a[0] = a0;                                                 /* 0x178736 lea (0x42b4,A5),A0 */
    return 0;
}
