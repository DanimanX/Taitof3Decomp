#include "fn_010352.h"
#include "../enemy/obj_1194C2.h"

/* ROM 0x010352: title-screen credit line.
 *
 * 0x010352-0x010360: tst.b A5-0x5DC1 (a status byte) ; bne 0x0103D6 and
 * btst #7,A5-0x144B must both take the "normal" branch for the credit counter
 * to be drawn. 0x0103B0-0x0103D4 then draws the counter.
 * 0x0103D6-0x010400 (FREE PLAY or CREDIT/ENJOY) is not translated: the
 * capture goes through 0x010376 -> bne 0x01039A, which needs 0x0001016 to
 * return non-zero, and the post image matches the 0x0103B0 path.
 * Text_RAM is 0x61CE3E on this screen, not 0x61CE4C (see fn_010352.c notes
 * in tests/test_fn.c). */
void Kernel_CreditLine010352(const f3mem_t *m)
{
    uint16_t d0;
    if (rd8(m, F3_A5 - 0x5DC1u) != 0u)             /* tst.b/bne 0x0103D6 */
        return;
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x80u) == 0u)   /* btst #7 ; beq 0x01039A */
        return;
    /* 0x01039A-0x0103B0 */
    Text_DrawString(m, 0x0001040Cu, 0x0061CE3Eu, 7u);
    /* 0x0103B4-0x0103D2: 2-digit decimal credit counter, blanked right to left */
    d0 = rd16(m, F3_A5 - 0x7F70u);
    Kernel_005666(m, d0 >> 8, 0x0061CE4Cu, 7u);
    Kernel_005666(m, d0, 0x0061CE4Cu, 1u);
}

/* ROM 0x005666: draw a NUL-terminated string as {col*2, char} pairs. */
uint32_t Kernel_005666(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t col)
{
    uint8_t d7 = (uint8_t)(col + col);
    for (;;) {
        uint8_t c = rd8(m, src++);
        if (c == 0u)
            break;
        wr8(m, dst++, d7);
        wr8(m, dst++, c);
    }
    return dst;
}
