/* ROM 0x1020B6: init child object stats/script (0x0CEDE4, 0x0D0678) and offset from parent (A0 = object). */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1020B6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x001086FCu);                                 /* 0x1020B6..0x1020BA */
    Obj_StartBehaviour(m, obj, 0x00102114u);                         /* 0x1020C0..0x1020C4 */
    r->d[0] = setw(r->d[0], rd16(m, 0x00102114u + 2u));

    uint32_t parent = rd32(m, obj + 0x08u);                          /* 0x1020CA */
    r->a[1] = parent;
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));                   /* 0x1020CE */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u)); /* 0x1020D4 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));                   /* 0x1020DA */
    wr32(m, obj + 0x00u, 0x001020E8u);                               /* 0x1020E0 */
    return 0;
}
