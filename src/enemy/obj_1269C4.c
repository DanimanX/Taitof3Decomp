#include "obj_1269C4.h"
#include "obj_11B262.h"

/* ROM 0x1269C4: steering tick. On countdown expiry retarget +0x54 = 0 with
 * a 0x3C-frame timer and hand over to 0x1269F2. */
int Obj_1269C4_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1269C4 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1269CA jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1269D0 jsr 0x11B262 */

    /* 0x1269D6: subq.w #1 -> bgt 0x1269BC tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001269C4u);         /* 0x1269BC */
        return 0;
    }
    /* 0x1269DC */
    wr8(m, obj + 0x54u, 0x00u);
    wr16(m, obj + 0x06u, 0x003Cu);                 /* 0x1269E2 */
    wr32(m, obj + 0x00u, 0x001269F2u);             /* 0x1269EA */
    return 0;
}
