#include "obj_126590.h"
#include "obj_11B262.h"

/* 0x1265A8: bfset (0x30EE,A5){bit:1} -- bfset with Dn bit = 0 sets */
static void bfset_bit(const f3mem_t *m, unsigned bit)
{
    uint32_t a = F3_A5 + 0x30EEu + bit / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (bit % 8u))));
}

/* ROM 0x126590: steering tick; on +0x06 expiry set A5+0x30EE{4} and hand
 * over to 0x1265BE with a fresh 0xA0 timer. */
int Obj_126590_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x126590 */
    ObjHelper_11B2CA(m, obj);                          /* 0x126596 */
    ObjHelper_11B262(m, obj);                          /* 0x12659C */

    /* 0x1265A2: subq.w #1 -> bgt 0x126588 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126590u);             /* 0x126588 */
        return 0;
    }

    bfset_bit(m, 0x04u);                               /* 0x1265A8 */
    wr16(m, obj + 0x06u, 0x00A0u);                     /* 0x1265AE */
    /* 0x1265B4 beq -> 0x1265D6: move.w only leaves Z set for a stored zero,
     * so the ROM takes the fall-through to 0x1265B6. */
    wr32(m, obj + 0x00u, 0x001265BEu);                 /* 0x1265B6 */
    return 0;
}
