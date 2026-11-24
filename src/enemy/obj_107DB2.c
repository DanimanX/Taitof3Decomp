#include "obj_107DB2.h"

/* ROM 0x0CEDE4: movement init from the record at A1 (no slot record).
 *   0CEDE4  lea (0x14,A1),A1
 *   0CEDE8  clr.l (0x1c,A0) / (0x24,A0)
 *   0CEDF0  move.w (A1)+,(0x24,A0)
 *   0CEDF8  move.w (A1)+,(0x28,A0)
 *   0CEDFC  move.l (A1)+,(0x2c,A0)
 *   0CEE00  move.l (A1),(0x30,A0)      (A1 not advanced)
 *   0CEE04  clr.w (0x34,A0) ; bset.b #1,(0x34,A0)
 *   0CEE0E  clr.w (0x6,A0) / (0x20,A0) / (0x22,A0)
 *   0CEE1A  move.l (0x14,A0),(0x3a,A0) ; move.l (0x18,A0),(0x3e,A0)
 *   0CEE26  clr.b (0x4f,A0) */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0CFA7C: frame record A1 + table 0x0CFA96[d0] -> sprite + hit box.
 *   0CFA7C  move.w (0x0cfa96,PC,D0w*0x2),D0w   (sign-extended index)
 *   0CFA84  adda.w D0w,A1
 *   0CFA86  move.w (A1)+,D0w ; move.b D0b,(0x1c,A0)
 *   0CFA8C  move.l (A1)+,(0x10,A0) ; move.l (A1),(0x68,A0) */
static void sub_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    int16_t off = (int16_t)rd16(m, 0x000CFA96u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    a1 += (uint32_t)off;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x107DB2: spawn just right and above the parent (+0x50), then fall into
 * 0x107E00 with a 0x00870000 countdown and script frame picked by +0x58. */
int Obj_107DB2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 5u)); /* addq.w #5 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x23u));

    /* 0x107DCC  lea (0x216,PC),A1 -> 0x107FE4 (record body 0x107FF8) */
    sub_0CEDE4(m, obj, 0x00107FE4u);
    wr32(m, obj + 0x58u, 0x00870000u);
    /* 0x107DE2  move.w (0x58,A0),D0w ; lea (0x224,PC),A1 -> 0x108004 */
    sub_0CFA7C(m, obj, 0x00108004u, rd16(m, obj + 0x58u));
    wr8(m, obj + 0x06u, 0x01u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u)); /* bclr.b #1 */
    wr32(m, obj + 0x00u, 0x00107E00u);
    return 0;
}
