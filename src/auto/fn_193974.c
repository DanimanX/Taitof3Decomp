/* ROM 0x193974: boss sub-part/head init (A0 = object, +0x0C = part id).
 * Stores low byte of +0x0C into +0x22 and loads stats from 0x194B82 via
 * 0x1848D6. For id == 0x0A, starts 0x179A5C anim from 0x194D28 and reparents
 * +0x08 to parent->+0x08; otherwise starts anim from 0x194D4E. Arms 0x1939EC. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* Run 0x179A5C init entry and update D0/A1 as ROM 0x179550 does for index 0. */
static int anim_init_regs(const f3mem_t *m, uint32_t obj, uint16_t idx,
                          uint32_t *d0_io, uint32_t *a1_out)
{
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;
    *d0_io = (*d0_io & 0xFFFF0000u) | idx;
    *a1_out = rd32(m, obj + 0x3Au) + 6u;                         /* 0x179550: (a1)+.w, (a1)+.l */
    return 0;
}

int Fn_193974(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = rd32(m, obj + 0x0Cu);                          /* 0x193974 */
    wr8(m, obj + 0x22u, (uint8_t)d0);                            /* 0x193978 */

    uint32_t rec = ((uint8_t)d0 == 0x0Au) ? 0x00194D28u : 0x00194D4Eu;
    uint16_t idx = rd16(m, rec);
    uint32_t body = rec + 2u;

    Obj_1848D6(m, obj, 0x00194B82u);                             /* 0x19398C / 0x1939C2 */
    wr16(m, obj + 0x40u, idx);                                   /* 0x19399A / 0x1939D0 */
    wr32(m, obj + 0x3Au, body);                                  /* 0x19399E / 0x1939D4 */

    uint32_t a1 = body;
    if (anim_init_regs(m, obj, idx, &d0, &a1) < 0)               /* 0x1939AC / 0x1939E2 */
        return -1;

    if (rd8(m, obj + 0x22u) == 0x0Au) {                          /* 0x19397C..0x193982 */
        uint32_t a4 = rd32(m, obj + 0x08u);                      /* 0x1939AE */
        wr32(m, obj + 0x08u, rd32(m, a4 + 0x08u));               /* 0x1939B2 */
    }

    wr32(m, obj + 0x00u, 0x001939ECu);                           /* 0x1939E4 */
    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
