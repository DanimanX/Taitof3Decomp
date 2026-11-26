/* ROM 0x18E656: periodic explosion/effect spawner tick (A0 = object).
 * Counts down +0x06 and re-arms 0x18E656 while > 0. On 0, copies parent
 * (+0x08)->+0x50/+0x54 to +0x50/+0x54, spawns 0x17FE40 (group 9, count 2,
 * arg 0xE51C0000), subtracts +0x42 from +0x62, and if > 0 and A5+0x4B38 != 0
 * reloads +0x06 = +0x42 and re-arms 0x18E656; otherwise sets +0x06 = 2 and
 * transitions to 0x18E196. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_18E656(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a4 = r->a[4];

    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x18E656 */
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer > 0) {                               /* 0x18E65A */
        wr32(m, obj, 0x0018E656u);                          /* 0x18E64E */
        return 0;
    }

    for (;;) {
        a4 = rd32(m, obj + 0x08u);                          /* 0x18E65C */
        wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));          /* 0x18E660 */
        wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));          /* 0x18E666 */
        d0 = Obj_Spawn(m, obj, 0x0017FE40u, 9u, 2u, 0xE51C0000u); /* 0x18E66C..0x18E686 */

        uint16_t step = rd16(m, obj + 0x42u);               /* 0x18E68A */
        d0 = setw(d0, step);
        uint16_t rem = (uint16_t)(rd16(m, obj + 0x62u) - step); /* 0x18E68E */
        wr16(m, obj + 0x62u, rem);
        if ((int16_t)rem <= 0)                              /* 0x18E692 */
            break;

        if (rd16(m, F3_A5 + 0x4B38u) == 0u)                 /* 0x18E63C..0x18E642 */
            break;
        wr16(m, obj + 0x06u, step);                         /* 0x18E646 */
        if (step != 0u) {                                   /* 0x18E64C */
            wr32(m, obj, 0x0018E656u);                      /* 0x18E64E */
            r->d[0] = d0;
            r->a[4] = a4;
            return 0;
        }
    }

    wr16(m, obj + 0x06u, 2u);                               /* 0x18E186 */
    wr32(m, obj, 0x0018E196u);                              /* 0x18E18E */
    r->d[0] = d0;
    r->a[4] = a4;
    return 0;
}
