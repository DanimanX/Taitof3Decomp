/* ROM 0x0DF72C: initialize chain-segment sub-part from 0x0DC84E / 0x0DF796 -> 0x0DF766. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0DF72C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];

    Obj_0CEDE4(m, obj, 0x000DC84Eu);                    /* 0x0DF72C..0x0DF730 */

    uint32_t a1 = 0x000DF796u;                          /* 0x0DF736 */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0DF73A */
    d0 = setw(d0, rd16(m, a1 + 2u));

    a1 = rd32(m, obj + 0x08u);                          /* 0x0DF740 */
    a1 = rd32(m, a1 + 0x08u);                           /* 0x0DF744 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x3Au));          /* 0x0DF748 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x000Eu)); /* 0x0DF74E */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));          /* 0x0DF754 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0006u)); /* 0x0DF75A */
    wr32(m, obj + 0x00u, 0x000DF766u);                  /* 0x0DF75E */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
