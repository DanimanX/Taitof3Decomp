#include "obj_12645E.h"
#include "obj_11B262.h"

/* ROM 0x12645E: steering tick; on +0x06 expiry, if the counter A5+0x30DA is
 * not above 0x7D0 run the family's 0x1266A8 dead end, otherwise aim at
 * 0x120/0x70 with a 0x50-frame timer and hand over to 0x12649C. */
int Obj_12645E_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x12645E */
    ObjHelper_11B2CA(m, obj);                          /* 0x126464 */
    ObjHelper_11B262(m, obj);                          /* 0x12646A */

    /* 0x126470: subq.w #1 -> bgt 0x126456 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0012645Eu);             /* 0x126456 */
        return 0;
    }

    /* 0x126476: cmpi.w #0x7D0,(0x30DA,A5); ble.w 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x07D0) {
        wr32(m, F3_A5 + 0x30EEu, 0u);                  /* 0x1266A8 */
        wr8(m, obj + 0x54u, 0x00u);
        wr16(m, obj + 0x06u, 0x0078u);                 /* 0x1266B2 */
        wr32(m, obj + 0x00u, 0x001266C2u);             /* 0x1266BA */
        return 0;
    }

    wr16(m, obj + 0x50u, 0x0120u);                     /* 0x126480 */
    wr16(m, obj + 0x52u, 0x0070u);
    wr16(m, obj + 0x06u, 0x0050u);                     /* 0x12648C */
    wr32(m, obj + 0x00u, 0x0012649Cu);                 /* 0x126494 */
    return 0;
}
