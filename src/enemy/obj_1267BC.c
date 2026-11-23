#include "obj_1267BC.h"
#include "obj_11B262.h"

/* ROM 0x1267BC: steering tick. On countdown expiry retarget +0x54 = 3, set
 * flag A5+0x30EE{0x11}, arm a 0x8C-frame timer and hand over to 0x1267F0. */
int Obj_1267BC_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1267BC jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1267C2 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1267C8 jsr 0x11B262 */

    /* 0x1267CE: subq.w #1 -> bgt 0x1267B4 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001267BCu);         /* 0x1267B4 */
        return 0;
    }
    /* 0x1267D4 */
    wr8(m, obj + 0x54u, 0x03u);
    {
        /* bfset (0x30EE,A5){0x11:1}: bit 1 of the long word at +0x30EE */
        uint32_t a = F3_A5 + 0x30EEu + 0x11u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x11u % 8u))));
    }
    wr16(m, obj + 0x06u, 0x008Cu);                 /* 0x1267E0 */
    wr32(m, obj + 0x00u, 0x001267F0u);             /* 0x1267E8 */
    return 0;
}
