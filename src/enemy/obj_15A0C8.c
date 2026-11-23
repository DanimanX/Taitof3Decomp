#include "obj_15A0C8.h"

/* ROM 0x15A0C8
 *
 * Bytes re-decoded from work/maincpu.bin:
 *   317c 00ff 0044             move.w #0xff,(0x44,A0)
 *   48e7 0080                  movem.l {A0},-(SP)
 *   3028 0044 0240 001f        move.w (0x44,A0),D0w; andi.w #0x1f,D0w
 *   6600 006c                  bne.w 0x15A148
 *   3028 0044 ea40 4440 5040  move.w (0x44,A0),D0w; asr.w #5; neg.w; addq.w #8
 *   323c 0004                  move.w #4,D1w
 *   48e7 ff80                  movem.l {A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
 *   41f9 0062 6200 d0ed a2aa  lea (0x626200).l,A0; adda.w (-0x5d56,A5),A0
 *   ...                        0x15A138..0x15A140: fill = ~((d1<<12)|((8-d1)<<4)
 *                              |(d0<<8)|(8-d0)) written to the 0xE8 words of
 *                              row 0x626200
 *   4cdf 0100                  movem.l (SP)+,{A0}
 *   20bc 0015 a154             move.l #0x15a154,(A0)
 *   4e75                       rts
 *
 * Branch flags: the `bne.w` at 0x15A0DA consumes the andi.w (a real test).
 * The two `ble.w` at 0x15A102/0x15A112 are `6f 00 0006` and always taken
 * (the previous instruction is a move.w -> Z=0 -> "greater"), and the
 * `beq.b 0x15A148` after the movem.l/lea/adda.w is `67 20` and never taken
 * (those instructions clear Z). So the clamp stores are dead code and the
 * row write happens whenever (0x44 & 0x1f) == 0. */
int Obj_15A0C8_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x44u, 0x00FFu);

    if ((rd16(m, obj + 0x44u) & 0x001Fu) != 0u) {
        wr32(m, obj + 0x00u, 0x0015A154u);   /* 0x15A148 */
        return 0;
    }

    /* 0x15A0DE: move.w (0x44,A0),D0w; asr.w #5; neg.w; addq.w #8 */
    uint16_t d0w = (uint16_t)(-(int16_t)(uint16_t)(rd16(m, obj + 0x44u) >> 5) + 8u);
    uint16_t d1w = 4u;

    uint32_t a0 = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    d1w &= 0x000Fu;                          /* andi.w #0xf,D1w */
    d0w &= 0x000Fu;                          /* andi.w #0xf,D0w (post-lea) */

    /* 0x15A11A: move.w #8,-(SP); sub.w D1w,(SP); lsl.w #8,D1w; add.w (SP)+,D1w;
     *           lsl.w #4,D1w
     * 0x15A126: same shape for D0w
     * 0x15A130: or.w D0w,D1w; not.w D1w */
    uint16_t t1 = (uint16_t)((d1w << 8) | (uint16_t)(8u - d1w));
    t1 = (uint16_t)(t1 << 4);
    uint16_t t0 = (uint16_t)((d0w << 8) | (uint16_t)(8u - d0w));
    uint16_t fill = (uint16_t)~(uint16_t)(t1 | t0);

    /* 0x15A134: move.w #0xe7,D7w; 0x15A138: move.w D7w,D0w; add.w D0w,D0w;
     * move.w D1w,(0,A0,D0w*1); dbf D7w */
    for (uint32_t d7 = 0u; d7 <= 0xE7u; d7++)
        wr16(m, a0 + d7 * 2u, fill);

    wr32(m, obj + 0x00u, 0x0015A154u);       /* 0x15A148 */
    return 0;
}
