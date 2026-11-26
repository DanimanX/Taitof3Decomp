#include "obj_BD47E.h"
#include "../objects/obj_shared_ext.h"

/* 68k divs.w: (remainder << 16) | quotient; on overflow (or a zero divisor,
 * which never happens here) the destination is left unchanged. */
static uint32_t divs_w(uint32_t n, uint16_t d)
{
    if ((int16_t)d == 0)
        return n;
    int32_t q = (int32_t)n / (int32_t)(int16_t)d;
    int32_t r = (int32_t)n % (int32_t)(int16_t)d;
    if (q < -32768 || q > 32767)
        return n;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* 68k divs.w where the first operand is an immediate: V only on quotient
 * overflow, and the destination is then just that immediate. */
static uint32_t divs_w_imm(int32_t n, uint16_t d)
{
    if ((int16_t)d == 0)
        return 0xFFFFu;    /* divide-by-zero trap; not reachable here */
    int32_t q = n / (int32_t)(int16_t)d;
    int32_t r = n % (int32_t)(int16_t)d;
    if (q < -32768 || q > 32767)
        return 0xFFFFu;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* ROM 0x176198: fill A5+0x1D0 - 0x5D56 + 0x40 words with D0 (evenly split
 * between the two memory half-lines when it is stepped 0..0x40). */
static void sub_176198(const f3mem_t *m, uint32_t a0, uint16_t d0w)
{
    uint32_t a = a0 + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    a += 0x1D0u;
    for (int i = 0; i < 0x40; i++) {
        wr16(m, a, d0w);
        a -= 2u;
    }
}

/* ROM 0x176832: line-RAM zoom ramp. Lines D7..D6 of the tables at A0 (word,
 * 0x3F ^ zoom) and A1 (byte, scale), both offset by A5-0x5D56. */
void sub_176832(const f3mem_t *m, uint32_t a0, uint32_t a1,
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
        uint32_t d0 = divs_w_imm(-0xA000, (uint16_t)(0x00A0u + d2w));
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
void sub_1768CA(const f3mem_t *m, uint32_t a0, uint16_t d7w, uint16_t d6w,
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

/* ROM 0x0BD47E: stage-entry line-RAM / playfield-window setup. Clears the two
 * longs at A5+0x4234, ramps the zoom and scroll of the 0x628400/0x62A400 and
 * 0x624400 windows over 0xE8 lines, blanks the two 0x62B400 table lines with
 * D0 = 0xB008 (move.w #-0x4FF8), then re-arms at 0x0BD544. */
int Obj_BD47E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BD47E: lea (0x4234,A5),A0 ; push 0,0 ; jsr 0x16AB26 */
    Obj_16AB26(m, F3_A5 + 0x4234u, 0u, 0u);

    /* 0x0BD49C: zoom ramp. Frame: (8)=0x62A400 A0, (0xC)=0x628400 A1,
     * (0x10).w=0 D7, (0x12).w=0xE7 D6, (0x14)=0xA00000 -> A5+0x4298,
     * (0x18)=0 D5, (0x1C)=0 D3, (0x20).w=2 -> A5+0x42A0 */
    sub_176832(m, 0x0062A400u, 0x00628400u, 0x0000u, 0x00E7u, 0x00A00000u,
               0u, 0u, 0x0002u);

    /* 0x0BD4D0: scroll ramp, first direction. Frame: (8)=0x624400 A0,
     * (0xC).w=0x74 D7, (0xE).w=0 D6, (0x10)=0 -> A5+0x4298, (0x14)=0x8000 D3,
     * (0x18)=0 D4 */
    sub_1768CA(m, 0x00624400u, 0x0074u, 0x0000u, 0u, 0x00008000u, 0u);

    /* 0x0BD4FA: scroll ramp, second direction: D7=0x74, D6=0xE7,
     * A5+0x4298=0, D3=-0x8000, D4=0 */
    sub_1768CA(m, 0x00624400u, 0x0074u, 0x00E7u, 0u, 0xFFFF8000u, 0u);

    /* 0x0BD524: move.w #-0x4FF8,-(SP) (D0 = 0xB008) ; pea 0x62B400 ;
     * jsr 0x176198 */
    sub_176198(m, 0x0062B400u, 0x0000B008u);

    /* 0x0BD538: movem.l (SP)+,{A0} ; move.l #0xBD544,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x000BD544u);
    return 0;
}
