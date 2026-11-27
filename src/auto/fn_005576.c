#include "fn_auto.h"
/* ROM 0x005576: UNVERIFIED (weak capture): blit a word tilemap, clearing source attributes before applying the mode. */
#include "fn_call_frames.h"

int Fn_005576(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t src = rd32(m, sp + 4u);
    uint32_t dst = rd32(m, sp + 8u);
    uint16_t flags = rd16(m, sp + 12u);
    uint16_t index = (uint16_t)(flags << 1);
    index = (uint16_t)((index & 0xFF00u) | (uint8_t)(index << 6));
    uint32_t handler = rd32(m, 0x005616u + (index >> 5));

    /* 0x005626..0x005664: all four entries in the direction table. */
    int16_t column_step;
    int16_t row_step;
    switch (handler) {
    case 0x005626u: column_step =  2; row_step =  0x80; break;
    case 0x005630u: column_step = -2; row_step =  0x80; break;
    case 0x00563Eu: column_step =  2; row_step = -0x80; break;
    case 0x005650u: column_step = -2; row_step = -0x80; break;
    default: return -1;                            /* mode outside the ROM table */
    }

    push_long(m, r, r->a[6]);                       /* 0x005576 LINK */
    for (unsigned i = 3; i > 0;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 7; i > 0;)
        push_long(m, r, r->d[--i]);
    int32_t rows = (int16_t)rd16(m, src);
    int32_t columns = (int16_t)rd16(m, src + 2u);
    src += 4u;
    push_long(m, r, 0x0055A4u);
    if (column_step < 0)                            /* extension $12FE */
        dst += (uint32_t)(2 * columns - 2);
    if (row_step < 0)
        dst += (uint32_t)(int32_t)(int16_t)(uint16_t)((rows - 1) * 0x80);
    r->a[7] += 4u;                                   /* direction handler RTS */

    uint16_t attr = (uint16_t)(flags << 8);
    do {
        uint32_t row_start = dst;
        int32_t remaining = columns;
        do {
            uint16_t tile = rd16(m, src);
            src += 2u;
            /* 0x0055AA BFCLR {17:6} clears bits 14..9 of the word. */
            wr16(m, dst, (uint16_t)((tile & 0x81FFu) ^ attr));
            dst += (uint32_t)(int32_t)column_step;
        } while (--remaining > 0);
        dst = row_start + (uint32_t)(int32_t)row_step;
    } while (--rows > 0);

    for (unsigned i = 0; i < 7; ++i, r->a[7] += 4u)
        r->d[i] = rd32(m, r->a[7]);
    for (unsigned i = 0; i < 3; ++i, r->a[7] += 4u)
        r->a[i] = rd32(m, r->a[7]);
    r->a[6] = rd32(m, r->a[7]);                       /* 0x0055C4 UNLK */
    r->a[7] = sp + 4u;                               /* 0x0055C6 RTS */
    return 0;
}
