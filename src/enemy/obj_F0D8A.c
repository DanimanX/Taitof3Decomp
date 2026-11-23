#include "obj_F0D8A.h"
#include "../objects/obj_shared.h"

/* ROM 0x0CEC4C: base state + the per-group script chunk at A1 */
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
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 0x05u)
        d0 = (uint16_t)(d0 + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0 * 4u);
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

/* ROM 0x0CEE2C */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = rd8(m, obj + 0x4Eu);
    uint8_t b = rd8(m, 0x000CEE40u + (uint32_t)d0);
    wr8(m, obj + 0x1Du, b);
    wr8(m, obj + 0x4Fu, b);
}

/* ROM 0x0CF1D8 */

/* ROM 0x0D0050 */
static void sub_0D0050(const f3mem_t *m, uint32_t obj)
{
    wr32(m, rd32(m, obj + 0x0Cu) + 0x28u, 0u);
}

/* ROM 0x0F0D8A */
int Obj_F0D8A_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CEC4C(m, obj, 0x000F1046u);
    sub_0CEE2C(m, obj);
    Obj_StartScript(m, obj, 0x000F107Eu);
    sub_0D0050(m, obj);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000F0DB8u);
    return 0;
}
