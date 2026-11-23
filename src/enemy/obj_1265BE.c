#include "obj_1265BE.h"
#include "obj_11B262.h"

/* ROM 0x1265BE: leader follow; on +0x06 expiry, if the counter above 0x7D0 is
 * not live, run the shared 0x1266A8 dead end; otherwise set A5+0x30EE{0x4},
 * aim at 0x50/0xB0 with a 0xC8-frame timer and hand over to 0x126608. */
int Obj_1265BE_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x1265BE */
    ObjHelper_11B2CA(m, obj);                          /* 0x1265C4 */
    ObjHelper_11B262(m, obj);                          /* 0x1265CA */

    /* 0x1265D0: subq.w #1 -> bgt 0x1265B6 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001265BEu);             /* 0x1265B6 */
        return 0;
    }
    /* 0x1265D6: cmpi.w #0x7D0,(0x30DA,A5); ble 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x07D0) {
        wr32(m, F3_A5 + 0x30EEu, 0u);                  /* 0x1266A8 clr.l */
        wr8(m, obj + 0x54u, 0x00u);                    /* 0x1266AC */
        wr16(m, obj + 0x06u, 0x0078u);                 /* 0x1266B2; Z clear */
        wr32(m, obj + 0x00u, 0x001266C2u);             /* 0x1266BA */
        return 0;
    }
    /* 0x1265E0: bfset (0x30EE,A5){0x4:1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x04u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x04u % 8u))));
    }
    wr8(m, obj + 0x54u, 0x05u);                        /* 0x1265E6 */
    wr16(m, obj + 0x50u, 0x0050u);                     /* 0x1265EC */
    wr16(m, obj + 0x52u, 0x00B0u);                     /* 0x1265F2 */
    wr16(m, obj + 0x06u, 0x00C8u);                     /* 0x1265F8; Z clear */
    wr32(m, obj + 0x00u, 0x00126608u);                 /* 0x126600 */
    return 0;
}
