#include "obj_E101A.h"

/* ROM 0x0CEC4C: base state + the per-group script chunk at A1 */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);                  /* clr.l (0x1c,A0) */
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));  /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    /* 0x0CEC92: group 5 keeps its own chunk, others use group+0x20 */
    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 0x05u)
        d0 = (uint16_t)(d0 + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0 * 4u);
    wr32(m, obj + 0x0Cu, a2);

    /* 0x0CECB2 */
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

/* ROM 0x0CFA86: first frame {spr.w, sprdef.l, hit.l} */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0E101A */
int Obj_E101A_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CEC4C(m, obj, 0x000DC84Eu);   /* lea (-0x47ce,PC) at 0xE101A */
    sub_0CFA86(m, obj, 0x000E1068u);   /* lea (0x42,PC) at 0xE1024 */
    wr32(m, obj + 0x00u, 0x000E1036u);
    return 0;
}
