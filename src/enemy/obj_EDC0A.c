#include "obj_EDC0A.h"
#include "../objects/obj_shared.h"

/* Shared 0x0C-call-site routines used by the 0x0EDCxx family. Bodies lifted
 * verbatim from the already-verified copies (src/enemy/obj_F40EA.c for
 * 0x0CEC4C / 0x0CEE2C, src/enemy/obj_F44B4.c for 0x0CF1D8). */

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

/* ROM 0x0CEE2C: colour index from A0+0x4E via the byte table 0x0CEE40 */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = rd8(m, obj + 0x4Eu);
    uint8_t c = rd8(m, 0x000CEE40u + (uint32_t)d0);
    wr8(m, obj + 0x1Du, c);
    wr8(m, obj + 0x4Fu, c);
}

/* ROM 0x0D0050: clear the dword at (A0+0xC struct)+0x28 */
static void sub_0D0050(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, a1 + 0x28u, 0u);
}

/* ROM 0x0EDC0A */
int Obj_EDC0A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0EDC0A: lea (0x2B6,PC),A1 -> 0x0EDC0C + 0x2B6 = 0x0EDEC2 */
    sub_0CEC4C(m, obj, 0x000EDEC2u);

    sub_0CEE2C(m, obj);                                     /* 0x0EDC14 */

    /* 0x0EDC1A: lea (0x2DE,PC),A1 -> 0x0EDC1C + 0x2DE = 0x0EDEFA */
    Obj_StartScript(m, obj, 0x000EDEFAu);                   /* jsr 0x0CF1D8 */

    sub_0D0050(m, obj);                                     /* 0x0EDC24 */

    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* bset.b #2 */
    wr32(m, obj + 0x00u, 0x000EDC38u);
    return 0;
}
