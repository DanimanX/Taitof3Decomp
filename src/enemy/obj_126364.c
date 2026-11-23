#include "obj_126364.h"
#include "obj_11B262.h"

/* ROM 0x126364: steering tick (0x1261AA family). Follow the leader and ease
 * the turn, then count +0x06 down; on expiry clear the byte at +0x54, re-arm
 * 0x1E and hand over to 0x126390. 0x126386 is a `beq` after
 * `move.w #0x1E,(0x6,A0)` whose flag producer the bundle truncates; the
 * golden pairs show the branch NOT taken, so it always falls into 0x126388. */
int Obj_126364_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126364 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x12636A jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126370 jsr 0x11B262 */

    /* 0x126376: subq.w #1 -> bgt 0x12635C tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126364u);         /* 0x12635C */
        return 0;
    }

    wr8(m, obj + 0x54u, 0x00u);                    /* 0x12637C: sf (0x54,A0) */
    wr16(m, obj + 0x06u, 0x001Eu);                 /* 0x126380 */
    wr32(m, obj + 0x00u, 0x00126390u);             /* 0x126388 */
    return 0;
}
