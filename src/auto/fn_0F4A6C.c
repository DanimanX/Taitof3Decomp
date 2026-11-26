/* ROM 0x0F4A6C: initialize enemy from 0x0F8214, script 0x0F4C58, sprite 0x0F4C82 -> 0x0F4AA6. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CFA86: load {pal.w, spr.l, hit.l} from A1 into +0x1C/+0x10/+0x68. */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t *a1_io, uint32_t *d0_io)
{
    uint32_t a1 = *a1_io;
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

int Fn_0F4A6C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1;

    Obj_0CEDE4(m, obj, 0x000F8214u);                    /* 0x0F4A6C..0x0F4A70 */
    Obj_StartScript(m, obj, 0x000F4C58u);               /* 0x0F4A76..0x0F4A7A */

    a1 = 0x000F4C82u;                                   /* 0x0F4A80 */
    sub_0CFA86(m, obj, &a1, &d0);                       /* 0x0F4A84 */

    wr8(m, obj + 0x1Du, 0xDEu);                         /* 0x0F4A8A */
    wr16(m, obj + 0x1Eu, 0x8080u);                      /* 0x0F4A90 */

    uint8_t d0b = rd8(m, obj + 0x4Eu);                  /* 0x0F4A96 */
    d0 = (d0 & 0xFFFFFF00u) | d0b;
    wr8(m, obj + 0x4Eu, (uint8_t)(d0b + d0b));          /* 0x0F4A9A */

    wr32(m, obj + 0x00u, 0x000F4AA6u);                  /* 0x0F4A9E */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
