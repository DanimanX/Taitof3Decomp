#include "obj_10A024.h"

/* ROM 0x0CEC4C: movement init + per-slot path record from the record at A1.
 *   0CEC4C  lea (0x14,A1),A1
 *   0CEC50  clr.l (0x1c,A0) / (0x24,A0)
 *   0CEC58  move.w (A1)+,(0x24,A0)
 *   0CEC60  move.w (A1)+,(0x28,A0)
 *   0CEC64  move.l (A1)+,(0x2c,A0)
 *   0CEC68  move.l (A1)+,(0x30,A0)
 *   0CEC6C  clr.w (0x34,A0) ; bset.b #1,(0x34,A0)
 *   0CEC76  clr.w (0x6,A0) / (0x20,A0) / (0x22,A0)
 *   0CEC82  move.l (0x14,A0),(0x3a,A0) ; move.l (0x18,A0),(0x3e,A0)
 *   0CEC8E  clr.b (0x4f,A0)
 *   0CEC92  D0w = slot index (+0x20 when group != 5) -> table 0x0CECE4
 *   0CECE2  per-slot record: 4 x {timer.w, moveptr.l} + trailing moveptr.l */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        d0 = (uint16_t)(d0 + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0 * 4u); /* mulu.w */
    wr32(m, obj + 0x0Cu, a2);
    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));
}

/* ROM 0x0CEE2C: palette byte from table 0x0CEE40 indexed by obj+0x4E.
 *   0CEE2C  clr.w D0w ; move.b (0x4e,A0),D0b
 *   0CEE32  move.b (0x0cee40,PC,D0w*0x1),D0b  (sign-extended index)
 *   0CEE36  move.b D0b,(0x1d,A0) ; move.b D0b,(0x4f,A0) */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint8_t c = rd8(m, 0x000CEE40u + (uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x4Eu));
    wr8(m, obj + 0x1Du, c);
    wr8(m, obj + 0x4Fu, c);
}

/* ROM 0x10A024: init from the record at 0x10A306, then fall into the
 * 0x10A054 state with a 0x010E0000 countdown in +0x58. */
int Obj_10A024_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x10A024  lea (0x2dc,PC),A1 -> 0x10A302 (record body 0x10A316) */
    sub_0CEC4C(m, obj, 0x0010A302u);
    sub_0CEE2C(m, obj);
    wr32(m, obj + 0x58u, 0x010E0000u);
    wr32(m, obj + 0x60u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u)); /* bclr.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));           /* bset.b #2 */
    wr32(m, obj + 0x00u, 0x0010A054u);
    return 0;
}
