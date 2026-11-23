#include "obj_1264F8.h"
#include "obj_11B262.h"

/* ROM 0x1264F8: leader follow (0x11B288 / 0x11B2CA / 0x11B262); when the
 * +0x06 countdown expires it aims at 0x100/0x70 with a 0x78-frame timer and
 * hands over to 0x12652C. A positive count re-arms through 0x1264F0. */
int Obj_1264F8_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x1264F8 */
    ObjHelper_11B2CA(m, obj);                          /* 0x1264FE */
    ObjHelper_11B262(m, obj);                          /* 0x126504 */

    /* 0x12650A: subq.w #1 -> bgt 0x1264F0 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001264F8u);             /* 0x1264F0 */
        return 0;
    }
    wr16(m, obj + 0x50u, 0x0100u);                     /* 0x126510 */
    wr16(m, obj + 0x52u, 0x0070u);                     /* 0x126516 */
    wr16(m, obj + 0x06u, 0x0078u);                     /* 0x12651C; Z clear */
    if ((int16_t)0x0078 == 0) {                        /* 0x126522 beq 0x126544 */
        return 0;
    }
    wr32(m, obj + 0x00u, 0x0012652Cu);                 /* 0x126524 */
    return 0;
}
