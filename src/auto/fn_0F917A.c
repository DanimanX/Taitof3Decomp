/* ROM 0x0F917A: random debris/particle initializer (A0 = object).
 * Seeds HP/hitboxes from 0x0F824C (0x0CEDE4), picks one of 4 behaviour
 * records at 0x10238C via 0x0D0E3A (0x0D0678), randomizes X (+0x14) by
 * [-8..7] and Y velocity (+0x54) by [-7..-4], sets +0x1D = 0x5A, and
 * arms 0x0F91CA. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0F917A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    Obj_0CEDE4(m, obj, 0x000F824Cu);                    /* 0x0F917A..0x0F917E */

    uint16_t idx = (uint16_t)(Obj_Next4Way(m) & 3u);    /* 0x0F9184..0x0F918A */
    a1 = rd32(m, 0x0010238Cu + (uint32_t)idx * 4u);     /* 0x0F918E */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F9196 */
    a1 += 8u;

    uint16_t rx = (uint16_t)((Obj_Next4Way(m) & 0x0Fu) - 8u); /* 0x0F919C..0x0F91A6 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rx)); /* 0x0F91A8 */

    uint16_t vy = (uint16_t)((Obj_Next4Way(m) & 3u) - 7u); /* 0x0F91AC..0x0F91B6 */
    d0 = setw(d0, vy);
    wr16(m, obj + 0x54u, vy);                           /* 0x0F91B8 */
    wr8(m, obj + 0x1Du, 0x5Au);                         /* 0x0F91BC */
    wr32(m, obj, 0x000F91CAu);                          /* 0x0F91C2 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
