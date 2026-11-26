/* ROM 0x0F5718: initialize enemy from 0x0F51F2, random script 0x0F522A[0..3], behaviour 0x0F53F6 -> 0x0F5750. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0F5718(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];

    (void)Obj_SeedFromParams(m, obj, 0x000F51F2u);      /* 0x0F5718..0x0F571C */

    uint16_t idx = (uint16_t)(Obj_Next4Way(m) & 3u);    /* 0x0F5722..0x0F5728 */
    uint32_t a1 = rd32(m, 0x000F522Au + (uint32_t)idx * 4u); /* 0x0F572C */
    Obj_StartScript(m, obj, a1);                        /* 0x0F5732 */

    a1 = 0x000F53F6u;                                   /* 0x0F5738 */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F573C */
    d0 = setw(d0, rd16(m, a1 + 2u));
    a1 += 8u;

    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* 0x0F5742 */
    wr32(m, obj + 0x00u, 0x000F5750u);                  /* 0x0F5748 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
