#include "obj_F5CF6.h"
#include "../objects/obj_shared.h"

/* ROM 0x0CEC4C */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
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
    a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        d0 = (uint16_t)(d0 + 0x20u);

    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)(int32_t)(int16_t)d0 * 4u);
    wr32(m, obj + 0x0Cu, a2);
    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));
}

/* ROM 0x0D0678 */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0E3A */

/* ROM 0x0F5CF6 */
int Obj_F5CF6_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CEC4C(m, obj, 0x000F6028u);
    sub_0D0678(m, obj, 0x000F6060u);

    wr8(m, obj + 0x06u, 1u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));

    wr32(m, obj + 0x58u, 0x00960000u);
    wr32(m, obj + 0x5Cu, 0x00960000u);
    wr32(m, obj + 0x60u, 0x00960000u);
    wr32(m, obj + 0x64u, 0x00960000u);

    uint16_t d0 = Obj_Next4Way(m);
    wr16(m, obj + 0x50u, d0);
    if (d0 == 0u)
        d0 = 1u;
    wr8(m, obj + 0x22u, (uint8_t)d0);

    uint32_t parent = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x4Fu, rd8(m, parent + 0x4Fu));
    wr32(m, obj + 0x00u, 0x000F5D58u);
    return 0;
}
