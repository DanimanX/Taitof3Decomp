#include "obj_17E99A.h"
#include "../objects/obj_script10.h"

/* ROM 0x17E99A: script-driven projectile. It runs the 0x178982 movement script,
 * keeps +0x16/+0x1A as the delta it just travelled, and once it leaves the
 * 0x184A08 bounds it arms the +0x6 lifecycle timer (0x100) and parks on
 * 0x17E9F6, which releases the object when the timer runs out. */
int Obj_17E99A_Tick(const f3mem_t *m, uint32_t obj)
{
    if (Obj_Script10(m, obj) < 0)
        return -1;

    /* cmp2.w compares against a (lo,hi) pair; bcs means "inside the range" */
    {
        uint16_t lo = rd16(m, 0x184A08u), hi = rd16(m, 0x184A0Au);
        uint16_t x = rd16(m, obj + 0x50u);
        if (x >= lo && x <= hi)
            goto expire;
        uint16_t lo2 = rd16(m, 0x184A0Cu), hi2 = rd16(m, 0x184A0Eu);
        uint16_t y = rd16(m, obj + 0x54u);
        if (y >= lo2 && y <= hi2)
            goto expire;
    }

    {
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u));
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u));
        wr16(m, obj + 0x16u, dx);
        wr16(m, obj + 0x1Au, dy);
        wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
        wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    }
    wr32(m, obj + 0x00u, 0x0017E99Au);
    return 0;

expire:
    wr16(m, obj + 0x06u, 0x0100u);
    wr32(m, obj + 0x00u, 0x0017E9F6u);
    return 0;
}
