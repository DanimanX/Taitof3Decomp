/* ROM 0x12A606: delayed wave launcher. Decrements the word at +0x6 and, while
 * it stays > 0, re-arms 0x12A606; otherwise spawns 0x12B204 (group 3, 2
 * groups, arg 0x12C39C) and 0x12B4AA (group 3, 2 groups, arg A5+0x3296),
 * resets +0x28 = 0x10000, +0x24 = 0, +0x36.l = 0 and +0x6.b = 0, and arms
 * 0x12A666. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_12A606(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint16_t c = (uint16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12A606 subq.w */

    wr16(m, obj + 0x06u, c);
    if ((int16_t)c > 0) {                                     /* 0x12A60A bgt */
        wr32(m, obj + 0x00u, 0x0012A606u);                    /* 0x12A5FE */
        r->d[0] = d0;
        return 0;                                             /* 0x12A604 rts */
    }

    Obj_Spawn(m, obj, 0x0012B204u, 3u, 2u, 0x0012C39Cu);     /* 0x12A60C..0x12A620 */
    d0 = Obj_Spawn(m, obj, 0x0012B4AAu, 3u, 2u,
                   rd32(m, F3_A5 + 0x3296u));                 /* 0x12A62A..0x12A63C */

    wr32(m, obj + 0x28u, 0x00010000u);                        /* 0x12A646 */
    wr32(m, obj + 0x24u, 0x00000000u);                        /* 0x12A64E */
    wr32(m, obj + 0x36u, 0x00000000u);                        /* 0x12A656 clr.l */
    wr8(m, obj + 0x06u, 0u);                                  /* 0x12A65A sf */
    wr32(m, obj + 0x00u, 0x0012A666u);                        /* 0x12A65E */

    r->d[0] = d0;
    return 0;                                                 /* 0x12A664 rts */
}
