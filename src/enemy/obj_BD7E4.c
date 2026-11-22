#include "obj_BD7E4.h"
#include "../objects/obj_pool.h"

#define ASR16_BD7E4(v) ((int16_t)(v) < 0 ? (uint16_t)(((uint16_t)(v) >> 1) | 0x8000u) : (uint16_t)((uint16_t)(v) >> 1))

/* 68k divs.w: (remainder << 16) | quotient; on overflow the destination is
 * unchanged (V set). Divisor is never 0 here. */
static uint32_t divs_w(uint32_t n, uint16_t d)
{
    int32_t q = (int32_t)n / (int32_t)(int16_t)d;
    int32_t r = (int32_t)n % (int32_t)(int16_t)d;
    if (q < -32768 || q > 32767)
        return n;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* ROM 0x176832: line-RAM zoom ramp. Lines D7..D6 of the tables at A0
 * (word, 0x3F ^ zoom) and A1 (byte, scale), both offset by A5-0x5D56. */
static void sub_176832(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint16_t d7w, uint16_t d6w, uint32_t v4298,
                       uint32_t d5, uint32_t d3, uint16_t v42a0)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    a0 += off;
    a1 += off;
    int16_t d7 = (int16_t)d7w, d6 = (int16_t)d6w;
    wr32(m, F3_A5 + 0x4298u, v4298);
    wr16(m, F3_A5 + 0x42A0u, v42a0);
    int16_t d4 = 1;
    if (d7 > d6)
        d4 = -1;
    for (;;) {
        /* 0x176872 */
        uint16_t d2w = rd16(m, F3_A5 + 0x4298u);
        uint32_t d0 = divs_w(0xFFFF6000u, (uint16_t)(0x00A0u + d2w));
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0100u);
        d2w = (uint16_t)(0x0024u - rd16(m, F3_A5 + 0x42A0u));
        uint32_t d2 = (uint32_t)((int32_t)(int16_t)d2w * (int32_t)(int16_t)(uint16_t)d0);
        d2 <<= 6;
        d2 = divs_w(d2, 0x0032u);
        d2 ^= 0x3Fu;
        uint32_t idx = (uint32_t)((int32_t)d7 * 2);
        wr16(m, a0 + idx, (uint16_t)d2);
        wr8(m, a1 + idx, (uint8_t)d0);
        d5 += d3;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d5);
        d7 = (int16_t)(d7 + d4);
        if (d4 < 0) {
            if (!(d6 <= d7)) break;
        } else {
            if (!(d7 <= d6)) break;
        }
    }
}

/* ROM 0x1768CA: line-RAM scroll ramp: low 9 bits of the words D7..D6 at A0
 * (offset by A5-0x5D56) = high word of the accumulator A5+0x4298 */
static void sub_1768CA(const f3mem_t *m, uint32_t a0, uint16_t d7w, uint16_t d6w,
                       uint32_t v4298, uint32_t d3, uint32_t d4)
{
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    int16_t d7 = (int16_t)d7w, d6 = (int16_t)d6w;
    wr32(m, F3_A5 + 0x4298u, v4298);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {
        wr32(m, F3_A5 + 0x4298u, 0u - rd32(m, F3_A5 + 0x4298u));
        d3 = 0u - d3;
        d4 = 0u - d4;
    }
    int16_t d5 = 1;
    if (d7 > d6)
        d5 = -1;
    for (;;) {
        /* 0x17690C: bfins D2,D0{23:9} */
        uint16_t d2w = rd16(m, F3_A5 + 0x4298u);
        uint32_t p = a0 + (uint32_t)((int32_t)d7 * 2);
        uint16_t w = rd16(m, p);
        wr16(m, p, (uint16_t)((w & 0xFE00u) | (d2w & 0x01FFu)));
        d3 += d4;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d3);
        d7 = (int16_t)(d7 + d5);
        if (d5 < 0) {
            if (!(d6 <= d7)) break;
        } else {
            if (!(d7 <= d6)) break;
        }
    }
}

/* ROM 0x0BD7E4 */
int Obj_BD7E4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = ((uint32_t)rd16(m, obj + 0x60u) * 0x5000u) << 2;
    sub_176832(m, 0x0062A600u, 0x00628600u, 0x0000u, 0x00E7u, d0, 0u, 0u, 0x0003u);

    uint16_t d1 = (uint16_t)((uint16_t)ASR16_BD7E4(rd16(m, obj + 0x60u)) + 0x40u);
    d0 = 0x003FFFFFu;
    if (d1 == 0u)
        return -1; /* divide by zero trap */
    if (d0 / d1 <= 0xFFFFu)
        d0 = ((d0 % d1) << 16) | (d0 / d1);
    d0 &= 0xFFFFu;
    d0 = 0u - (d0 - 0xFFFFu);

    sub_1768CA(m, 0x00624600u, 0x0074u, 0x0000u, 0u, d0, 0u);
    sub_1768CA(m, 0x00624600u, 0x0074u, 0x00E7u, 0u, 0u - d0, 0u);

    uint16_t n = (uint16_t)(rd16(m, obj + 0x60u) + 1u);
    wr16(m, obj + 0x60u, n);
    if ((int16_t)n <= 0x58) {
        wr32(m, obj + 0x00u, 0x000BD7E4u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
