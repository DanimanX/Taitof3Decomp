/* ROM 0x0FAFE6: initialize tentacle segment sub-part from 0x0FB04A / 0x0FB06A -> 0x0FB002. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0FAFE6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];

    Obj_0CEDE4(m, obj, 0x000FB04Au);                    /* 0x0FAFE6..0x0FAFEA */

    uint32_t a1 = 0x000FB06Au;                          /* 0x0FAFF0 */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0FAFF4 */
    d0 = setw(d0, rd16(m, a1 + 2u));
    a1 += 8u;

    wr32(m, obj + 0x00u, 0x000FB002u);                  /* 0x0FAFFA */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
