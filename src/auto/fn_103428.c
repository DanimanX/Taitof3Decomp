/* ROM 0x103428: parent-linked attachment tick (A0 = object).
 * Copies parent (+0x08) +0x14/+0x18/+0x1D/+0x06 into +0x3A/+0x3E/+0x1D/+0x22;
 * if +0x22 == 0 calls Obj_StartDying (0x0CEE74). Otherwise sets (+0x14, +0x18) =
 * (+0x3A + 0x40, +0x3E + 0x10) and steps script via 0x0D0690. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

int Fn_103428(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x103428 */

    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x14u));            /* 0x10342C */
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x18u));            /* 0x103432 */
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));              /* 0x103438 */
    uint8_t active = rd8(m, a1 + 0x06u);
    wr8(m, obj + 0x22u, active);                          /* 0x10343E */
    if (active == 0u) {                                   /* 0x103444 */
        Obj_StartDying(m, obj);                           /* 0x0CEE74 */
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));           /* 0x10344A */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0040u)); /* 0x103450 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));           /* 0x103456 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0010u)); /* 0x10345C */
    step_script_regs(m, obj, &d0, &a1);                   /* 0x103462 -> 0x0D0690 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
