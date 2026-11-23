#include "obj_1267F0.h"
#include "obj_11B262.h"

/* ROM 0x1267F0: steering tick. On countdown expiry set flag A5+0x30EE{0},
 * arm a 0x1E-frame timer and hand over to the 0x12681E stage. */
int Obj_1267F0_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1267F0 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1267F6 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1267FC jsr 0x11B262 */

    /* 0x126802: subq.w #1 -> bgt 0x1267E8 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001267F0u);         /* 0x1267E8 */
        return 0;
    }
    /* 0x126808: bfset (0x30EE,A5){0:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x80u));
    wr16(m, obj + 0x06u, 0x001Eu);                 /* 0x12680E */
    wr32(m, obj + 0x00u, 0x0012681Eu);             /* 0x126816 */
    return 0;
}
