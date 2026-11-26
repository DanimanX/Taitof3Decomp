/* ROM 0x0E19F2: linked-list node tick (A0 = node, +0x08 = head, +0x50/+0x54 =
 * prev/next). While +0x22 != 0, tracks grandparent (+0x08->+0x08) Y + 0x26 *
 * table[+0x1E] via 0x0D4A12. Subtracts +0x0C from +0x14, steps script
 * (0x0D0690), and unlinks + dies (0x0CEE74) when +0x4C == 0. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

/* ROM 0x0D4A12: (0x0D4A5A[d1w] * d0w) >> 16 (swapped). */
static uint32_t sub_0D4A12(const f3mem_t *m, uint16_t d0w, uint16_t d1w)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)d1w * 4u);
    uint32_t prod = v * (uint32_t)d0w;
    return (prod << 16) | (prod >> 16);
}

int Fn_0E19F2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x22u) != 0u) {                          /* 0x0E19F2 */
        a1 = rd32(m, rd32(m, obj + 0x08u) + 0x08u);           /* 0x0E19FA..0x0E19FE */
        if (rd8(m, a1 + 0x06u) == 0u) {                       /* 0x0E1A02 */
            wr8(m, obj + 0x22u, 0u);                          /* 0x0E1A0A */
        } else {
            wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));        /* 0x0E1A12 */
            d1 = (d1 & 0xFFFF0000u) | (uint32_t)rd8(m, a1 + 0x1Eu); /* 0x0E1A1A..0x0E1A1C */
            d0 = sub_0D4A12(m, 0x0026u, (uint16_t)d1);        /* 0x0E1A18..0x0E1A20 */
            d1 = (d1 & 0xFFFF0000u) | 0x0026u;
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x0E1A26 */
        }
    }

    d0 = rd32(m, obj + 0x0Cu);                                /* 0x0E1A2A */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);          /* 0x0E1A2E */

    {
        uint16_t old_cnt = rd16(m, obj + 0x4Cu);
        Obj_StepScript(m, obj);                               /* 0x0E1A32 -> 0x0D0690 */
        if (old_cnt == 1u) {
            uint32_t scr = rd32(m, obj + 0x48u);
            uint16_t w = rd16(m, scr);
            if ((int16_t)w < 0) {
                d0 = (d0 & 0xFFFF0000u) | w;
                a1 = scr + 2u;
            } else {
                d0 = (d0 & 0xFFFF0000u) | rd16(m, scr + 2u);
                a1 = scr + 8u;
            }
        }
    }

    if (rd16(m, obj + 0x4Cu) == 0u) {                         /* 0x0E1A38 */
        a1 = rd32(m, obj + 0x50u);                            /* 0x0E1A42 */
        uint32_t a2 = rd32(m, obj + 0x54u);                   /* 0x0E1A46 */
        wr32(m, a1 + 0x54u, a2);                              /* 0x0E1A4A */
        if ((int32_t)a2 >= 0)                                 /* 0x0E1A4E */
            wr32(m, a2 + 0x50u, a1);                          /* 0x0E1A54 */
        Obj_StartDying(m, obj);                               /* 0x0E1A58 -> 0x0CEE74 */
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
