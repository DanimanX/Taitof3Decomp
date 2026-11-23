#include "obj_1263BE.h"
#include "obj_11B262.h"

/* ROM 0x1263BE: steering tick (0x1261AA family). The 0x11B288/0x11B2CA/
 * 0x11B262 trio, then the +0x06 countdown; on expiry A5+0x30DA > 0x7D0
 * starts the 0x126402 step (+0x50 = 0xD0, +0x52 = 0x50, +0x54 = 5, 0x14
 * timer), otherwise the family's 0x1266A8 dead end. 0x1263F8 is a `beq` with
 * a truncated flag producer; the golden pairs show the branch NOT taken, so
 * it always falls into 0x1263FA. */
int Obj_1263BE_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1263BE jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1263C4 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1263CA jsr 0x11B262 */

    /* 0x1263D0: subq.w #1 -> bgt 0x1263B6 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001263BEu);         /* 0x1263B6 */
        return 0;
    }

    /* 0x1263D6: cmpi.w #0x7D0,(0x30DA,A5); ble 0x1266A8 */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x07D0) {
        wr16(m, obj + 0x50u, 0x00D0u);             /* 0x1263E0 */
        wr16(m, obj + 0x52u, 0x0050u);             /* 0x1263E6 */
        wr8(m, obj + 0x54u, 0x05u);                /* 0x1263EC */
        wr16(m, obj + 0x06u, 0x0014u);             /* 0x1263F2 */
        wr32(m, obj + 0x00u, 0x00126402u);         /* 0x1263FA */
        return 0;
    }

    /* 0x1266A8: clr.l (0x30EE,A5); +0x54 = 0; 0x78 timer; -> 0x1266C2 */
    wr32(m, F3_A5 + 0x30EEu, 0u);
    wr8(m, obj + 0x54u, 0x00u);
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x001266C2u);
    return 0;
}
