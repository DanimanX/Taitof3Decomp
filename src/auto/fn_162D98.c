/* ROM 0x162D98: scrolling "wipe" band object tick (A0 = object).
 * +0x44 counts the band width down: the word 8-(+0x44>>4) and the constant 4
 * are clamped to 8 and packed as two 4-bit nibbles into a 0x0F0F-style mask
 * word (byte = 8-n | n<<8, then <<4, then both halves ORed and inverted), which
 * is written as 232 words from 0x626200 + A5-0x5D56 upwards (the band in
 * playfield RAM). +0x44 is then decremented: while it stays >= 0 the tick is
 * re-armed at 0x162D98, otherwise the object is freed (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"       /* Obj_Free */

#define BAND_BASE 0x00626200u
#define BAND_OFS  (F3_A5 - 0x5D56u)

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* move.w #8,-(SP) ; sub.w Dnw,(SP) ; lsl.w #8,Dnw ; add.w (SP)+,Dnw
 * (0x162DE2 adds a further `lsl.w #4` for the D1 half only) */
static uint16_t pack_nibble(uint16_t n)
{
    return (uint16_t)((uint16_t)(0x0008u - n) + (uint16_t)(n << 8));
}

int Fn_162D98(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    d0 = setw(d0, rd16(m, obj + 0x44u));                /* 0x162D98 */
    d0 = setw(d0, (uint16_t)((int16_t)d0 >> 4));        /* 0x162D9C asr.w */
    d0 = setw(d0, (uint16_t)(0u - (uint16_t)d0));       /* 0x162D9E neg.w */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 + 8u));       /* 0x162DA0 addq.w */
    d1 = setw(d1, 0x0004u);                             /* 0x162DA2 */

    uint32_t band = BAND_BASE + (uint32_t)(int32_t)(int16_t)rd16(m, BAND_OFS); /* 0x162DB4 */

    uint16_t n1 = (uint16_t)(d1 & 0x000Fu);             /* 0x162DB8 andi.w */
    if (n1 > 8u)                                        /* 0x162DBC cmpi/ble */
        n1 = 8u;                                        /* 0x162DC4 */
    uint16_t n0 = (uint16_t)(d0 & 0x000Fu);             /* 0x162DC8 */
    if (n0 > 8u)                                        /* 0x162DCC */
        n0 = 8u;                                        /* 0x162DD4 */

    uint16_t p1 = (uint16_t)(pack_nibble(n1) << 4);     /* 0x162DE2 lsl.w #4,D1w */
    uint16_t p0 = pack_nibble(n0);                      /* 0x162DEC (no shift) */
    uint16_t mask = (uint16_t)~(uint16_t)(p0 | p1);     /* 0x162DEE or.w / 0x162DF0 not.w */

    for (int16_t i = 0x00E7; i >= 0; i--) {             /* 0x162DF2..0x162DFE dbf */
        uint16_t off = (uint16_t)(i + i);               /* 0x162DF6..0x162DF8 */
        wr16(m, band + off, mask);                      /* 0x162DFA */
    }

    int16_t t = (int16_t)(rd16(m, obj + 0x44u) - 1u);   /* 0x162E0A subq.w */
    wr16(m, obj + 0x44u, (uint16_t)t);
    if (t >= 0)                                         /* 0x162E0E bge */
        wr32(m, obj + 0x00u, 0x00162D98u);              /* 0x162D90 */
    else
        Obj_Free(m, obj);                               /* 0x162E12 */

    r->d[0] = d0;                                       /* 0x162E02 movem restore */
    r->d[1] = d1;
    return 0;
}
