/* ROM 0x198584: phase timer for the boss (A0 = object). While +0x06 is above
 * zero it only re-arms itself; when it expires it spawns the 19A1A0 object
 * (group 3, one group scan), the 17A8D0 records 0x19BA3A and 0x19BA9A --
 * the 0x19BA3A object is published at A5+0x4BB4 -- sets +0x06 = 1 and moves
 * to state 0x1985F6. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_198584(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t v = (uint16_t)(rd16(m, obj + 0x06u) - 1u);      /* ROM 0x198584 subq.w #1,(0x6,A0) */

    wr16(m, obj + 0x06u, v);
    if ((int16_t)v > 0) {                                    /* ROM 0x198588 bgt.b 0x19857C */
        wr32(m, obj, 0x00198584u);                           /* ROM 0x19857C */
        return 0;                                            /* ROM 0x198582 rts */
    }

    r->d[0] = Obj_Spawn(m, obj, 0x0019A1A0u, 3u, 2u, 0u);     /* ROM 0x19858A..0x19859C */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BA3Au); /* ROM 0x1985A6..0x1985BA */
    wr32(m, F3_A5 + 0x4BB4u, r->d[0]);                       /* ROM 0x1985C4 */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BA9Au); /* ROM 0x1985C8..0x1985DC */

    wr16(m, obj + 0x06u, 0x0001u);                           /* ROM 0x1985E6 */
    /* ROM 0x1985EC beq.b 0x1985FC is never taken: move.w #1 sets Z = 0. */
    wr32(m, obj, 0x001985F6u);                               /* ROM 0x1985EE */
    return 0;                                                /* ROM 0x1985F4 rts */
}
