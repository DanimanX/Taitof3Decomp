#include "obj_12681E.h"
#include "obj_11B262.h"

/* ROM 0x12681E: steering tick. 0x11B288 turns +0x20 toward the leader,
 * 0x11B2CA eases +0x56 toward +0x54, 0x11B262 follows the leader; the
 * countdown at +0x06 then hands over to 0x126844 with a fresh 0xC8 timer. */
int Obj_12681E_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x12681E jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126824 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12682A jsr 0x11B262 */

    /* 0x126830: subq.w #1 -> bgt 0x126816 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0012681Eu);         /* 0x126816 */
        return 0;
    }
    wr16(m, obj + 0x06u, 0x00C8u);                 /* 0x126836 */
    wr32(m, obj + 0x00u, 0x00126844u);             /* 0x12683C */
    return 0;
}
