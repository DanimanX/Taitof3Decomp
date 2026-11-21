#include "obj_ECAC8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CEC4C: reset the object from the parameter block A1 and point
 * obj+0x0C at the per-slot sub-struct (table 0x0CECE4) */
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
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t d0w = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 0x05u)
        d0w = (uint16_t)(d0w + 0x20u);

    /* 0x0CECA6: A2 = table[d0w]; its 8/12/16/20/24 fields from A1 */
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0w * 4u);
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

/* ROM 0x0CEE2C: sub-struct frame byte from table 0x0CEE40[obj+0x4E] */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0w = rd8(m, obj + 0x4Eu);
    uint8_t b = rd8(m, 0x000CEE40u + (uint32_t)d0w);
    wr8(m, obj + 0x1Du, b);
    wr8(m, obj + 0x4Fu, b);
}

/* ROM 0x0D0050: clr.l (0x28,A1), A1 = obj+0x0C pointer */
static void sub_0D0050(const f3mem_t *m, uint32_t obj)
{
    wr32(m, rd32(m, obj + 0x0Cu) + 0x28u, 0u);
}

/* ROM 0xECAC8 */
int Obj_ECAC8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0ECAC8: lea (0x316,PC),A1 -> 0x0ECAEA (operand at 0x0ECACA) */
    sub_0CEC4C(m, obj, 0x000ECACAu + 0x316u);

    /* 0x0ECAD2 */
    sub_0CEE2C(m, obj);

    /* 0x0ECAD8: bclr.b #1,(0x34,A0) */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));

    /* 0x0ECADE */
    sub_0D0050(m, obj);

    /* 0x0ECAE4: Obj_Spawn(fn 0x0ECF0C, group 5, ngroups 2, arg 0) */
    uint32_t a1 = Obj_Spawn(m, obj, 0x000ECF0Cu, 5u, 2u, 0u);
    if ((int32_t)a1 >= 0) {
        /* 0x0ECB08 */
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) - 0x10u));
    }

    /* 0x0ECB1A */
    wr8(m, obj + 0x21u, 0x05u);
    wr8(m, obj + 0x23u, 0x04u);
    wr16(m, obj + 0x58u, 0x00B4u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* bset.b #2 */
    wr32(m, obj, 0x000ECB3Au);
    return 0;
}
