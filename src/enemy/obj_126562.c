#include "obj_126562.h"
#include "obj_11B262.h"

/* ROM 0x126562: leader follow; on +0x06 expiry publish the 0x30EE{0x3} flag
 * and arm 0x1E frames toward 0x126590. A positive count re-arms through
 * 0x12655A. */
int Obj_126562_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x126562 */
    ObjHelper_11B2CA(m, obj);                          /* 0x126568 */
    ObjHelper_11B262(m, obj);                          /* 0x12656E */

    /* 0x126574: subq.w #1 -> bgt 0x12655A tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126562u);             /* 0x12655A */
        return 0;
    }
    /* 0x12657A: bfset (0x30EE,A5){0x3:1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x03u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x03u % 8u))));
    }
    wr16(m, obj + 0x06u, 0x001Eu);                     /* 0x126580; Z clear */
    wr32(m, obj + 0x00u, 0x00126590u);                 /* 0x126588 */
    return 0;
}
