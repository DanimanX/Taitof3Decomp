#include "obj_F778A.h"

/* ROM 0x0CEDE4: common init; A1 points at the parameter block (+0x14 on) */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;

    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0CFA86: {flip.w, sprdef.l, selector.l} record at A1 */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0w);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));   /* move.l (A1),(0x68,A0) */
}

/* ROM 0x0F778A */
int Obj_F778A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x0Cu, rd16(m, obj + 0x12u));          /* move.w (0x12,A0),(0xC,A0) */
    /* 0x0F7790: lea (0x340,PC),A1 = 0x0F7AD2 */
    sub_0CEDE4(m, obj, 0x000F7AD2u);
    /* 0x0F779A: lea (0x386,PC),A1 = 0x0F7B22 */
    sub_0CFA86(m, obj, 0x000F7B22u);
    wr16(m, obj + 0x58u, 0x010Eu);
    wr16(m, obj + 0x60u, 0x000Au);
    wr32(m, obj + 0x64u, 0x00009C40u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u)); /* bclr.b #1 */
    wr8(m, obj + 0x06u, 0x01u);

    uint32_t a1 = rd32(m, obj + 0x08u);
    if ((int8_t)rd8(m, a1 + 0x4Eu) < 0) {                /* tst.b (0x4E,A1) */
        wr8(m, obj + 0x1Du, 0x6Eu);
        wr8(m, obj + 0x4Fu, 0x6Eu);
    }

    /* 0x0F77DC */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x001Du));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 1u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* bset.b #2 */
    wr32(m, obj, 0x000F7800u);
    return 0;
}
