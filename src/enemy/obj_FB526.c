#include "obj_FB526.h"
#include "../objects/obj_shared.h"
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

    /* 0x0CEC92: clr.w D0w; move.b (0x5,A0),D0b */
    uint16_t d0w = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 0x05u)
        d0w = (uint16_t)(d0w + 0x20u);

    /* 0x0CECA6: A2 = table[d0w]; then 8/12/16/20/24-layout from A1 */
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

/* ROM 0x0D0678: script pointer, first frame word and initial params */
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

/* ROM 0x0FB526: clr.w (0x10,A0), then the 0x0FB52A body shared with the
 * 0x0FCBC4/0x0FCBCE/0x0FCBD8 variants */
int Obj_FB526_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x10u, 0u);                               /* 0x0FB526 */
    return Obj_FB52A_Body(m, obj);
}

int Obj_FB52A_Body(const f3mem_t *m, uint32_t obj)
{
    /* 0x0FB52A: lea (0x7DA,PC),A1 -> 0x0FB52C + 0x7DA = 0x0FBD06 */
    sub_0CEC4C(m, obj, 0x000FBD06u);

    /* 0x0FB534..0x0FB546: the sub-struct A2 left in A1 by 0x0CEC4C */
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    wr16(m, a2 + 0x00u, 0x0708u);                           /* move.w #0x708,(A2) */
    wr16(m, a2 + 0x0Cu, 0u);                                /* clr.w (0xC,A2) */
    wr16(m, a2 + 0x10u, rd16(m, obj + 0x10u));              /* (0x10,A0) -> (0x10,A2) */
    wr16(m, a2 + 0x14u, 0u);                                /* clr.w (0x14,A2) */
    wr8(m, obj + 0x06u, 0x01u);                             /* move.b #1,(0x6,A0) */
    wr16(m, obj + 0x42u, 0x009Du);                          /* move.w #0x9D,(0x42,A0) */
    wr32(m, obj + 0x08u, 0x009D0000u);                      /* move.l #0x9D0000,(0x8,A0) */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x80u)); /* bset.b #7 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u)); /* bclr.b #1 */

    /* 0x0FB566: lea (0xEDE,PC),A1 -> 0x0FB568 + 0xEDE = 0x0FC446 */
    sub_0D0678(m, obj, 0x000FC446u);

    wr32(m, a2 + 0x20u, rd32(m, obj + 0x68u));              /* (0x68,A0) -> (0x20,A2) */

    Obj_DrawSprite(m, obj);                                 /* jsr 0x001282 */
    wr32(m, obj + 0x00u, 0x000FB584u);
    return 0;
}
