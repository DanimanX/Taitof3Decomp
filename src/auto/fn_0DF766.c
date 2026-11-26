/* ROM 0x0DF766: shrinking/scripted doubly-linked node tick (A0 = object).
 * Subtracts 2 from +0x14.w, steps script (0x0D0690), and if +0x4C.w == 0
 * unlinks A0 from doubly-linked list (+0x50 prev, +0x54 next) and starts
 * dying (0x0CEE74). */
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

int Fn_0DF766(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 2u)); /* 0x0DF766 */
    step_script_regs(m, obj, &r->d[0], &r->a[1]);            /* 0x0DF76A -> 0x0D0690 */
    if (rd16(m, obj + 0x4Cu) != 0u)                          /* 0x0DF770..0x0DF774 */
        return 0;

    uint32_t a1 = rd32(m, obj + 0x50u);                      /* 0x0DF77A */
    uint32_t a2 = rd32(m, obj + 0x54u);                      /* 0x0DF77E */
    r->a[1] = a1;
    r->a[2] = a2;
    wr32(m, a1 + 0x54u, a2);                                 /* 0x0DF782 */
    if ((int32_t)a2 >= 0)                                    /* 0x0DF786 */
        wr32(m, a2 + 0x50u, a1);                             /* 0x0DF78C */
    Obj_StartDying(m, obj);                                  /* 0x0DF790 -> 0x0CEE74 */
    return 0;
}
