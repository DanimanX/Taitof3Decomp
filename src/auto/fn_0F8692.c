/* ROM 0x0F8692: initialize orbiting-turret sub-part from 0x0F872A / 0x0F8762 -> 0x0F86B4. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0F8692(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];

    (void)Obj_SeedFromParams(m, obj, 0x000F872Au);      /* 0x0F8692..0x0F8696 */

    uint32_t a1 = 0x000F8762u;                          /* 0x0F869C */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F86A0 */
    d0 = setw(d0, rd16(m, a1 + 2u));
    a1 += 8u;

    wr16(m, obj + 0x58u, 0x00B4u);                      /* 0x0F86A6 */
    wr32(m, obj + 0x00u, 0x000F86B4u);                  /* 0x0F86AC */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
