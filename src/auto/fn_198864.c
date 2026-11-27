/* ROM 0x198864: phase timer for the boss (A0 = object). While +0x06 is above
 * zero it only re-arms itself; when it expires it spawns four 17A8D0 records
 * (group 3, one group scan) with the arguments 0x19BA4A, 0x19BACA, 0x19B9BA
 * and 0x19BADA, sets +0x06 = 1 and moves to state 0x1988F2. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_198864(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t v = (uint16_t)(rd16(m, obj + 0x06u) - 1u);      /* ROM 0x198864 subq.w #1,(0x6,A0) */

    wr16(m, obj + 0x06u, v);
    if ((int16_t)v > 0) {                                    /* ROM 0x198868 bgt.b 0x19885C */
        wr32(m, obj, 0x00198864u);                           /* ROM 0x19885C */
        return 0;                                            /* ROM 0x198862 rts */
    }

    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BA4Au); /* ROM 0x19886A..0x19887E */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BACAu); /* ROM 0x198888..0x19889C */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019B9BAu); /* ROM 0x1988A6..0x1988BA */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BADAu); /* ROM 0x1988C4..0x1988D8 */

    wr16(m, obj + 0x06u, 0x0001u);                           /* ROM 0x1988E2 */
    /* ROM 0x1988E8 beq.b 0x1988F8 is never taken: move.w #1 sets Z = 0. */
    wr32(m, obj, 0x001988F2u);                               /* ROM 0x1988EA */
    return 0;                                                /* ROM 0x1988F0 rts */
}
