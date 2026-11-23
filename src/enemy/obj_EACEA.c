#include "obj_EACEA.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x0CF1D8: script pointer obj+0x44 and flag bit 1 of obj+7 */

/* ROM 0x0D0678: install a script record: ptr A1, duration.w (obj+0x4C),
 * frame byte (obj+0x1C), sprite (obj+0x10), long (obj+0x68) */
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

/* ROM 0xEACEA */
int Obj_EACEA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0EACEA: lea (0x306,PC),A1 -> 0x0EACEA (operand at 0x0EACEC) */
    sub_0CEC4C(m, obj, 0x000EACECu + 0x306u);

    /* 0x0EACF4 */
    sub_0CEE2C(m, obj);

    /* 0x0EACFA: A1 = obj+0x10 */
    Obj_StartScript(m, obj, rd32(m, obj + 0x10u));

    /* 0x0EAD04 */
    wr8(m, obj + 0x22u, rd8(m, obj + 0x36u));

    /* 0x0EAD0A: lea (0x3AE,PC),A1 -> 0x0EAD0A */
    sub_0D0678(m, obj, 0x000EAD0Cu + 0x3AEu);

    /* 0x0EAD14 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u)); /* bclr.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));           /* bset.b #2 */
    wr32(m, obj, 0x000EAD28u);
    return 0;
}
