#include "obj_12652C.h"
#include "obj_11B262.h"

/* ROM 0x12652C: leader follow; on +0x06 expiry it drops the target byte,
 * arms 0x50 frames and hands over to 0x126562 (or to the shared 0x1266A8
 * dead end when the counter above 0x7D0 is not live). */
int Obj_12652C_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x12652C */
    ObjHelper_11B2CA(m, obj);                          /* 0x126532 */
    ObjHelper_11B262(m, obj);                          /* 0x126538 */

    /* 0x12653E: subq.w #1 -> bgt 0x126524 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0012652Cu);             /* 0x126524 */
        return 0;
    }
    /* 0x126544: cmpi.w #0x7D0,(0x30DA,A5); ble 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x07D0) {
        wr32(m, F3_A5 + 0x30EEu, 0u);                  /* 0x1266A8 clr.l */
        wr8(m, obj + 0x54u, 0x00u);                    /* 0x1266AC */
        wr16(m, obj + 0x06u, 0x0078u);                 /* 0x1266B2; Z clear */
        wr32(m, obj + 0x00u, 0x001266C2u);             /* 0x1266BA */
        return 0;
    }
    wr8(m, obj + 0x54u, 0x00u);                        /* 0x12654E sf */
    wr16(m, obj + 0x06u, 0x0050u);                     /* 0x126552; Z clear */
    wr32(m, obj + 0x00u, 0x00126562u);                 /* 0x12655A */
    return 0;
}
