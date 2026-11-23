#include "obj_12649C.h"
#include "obj_11B262.h"

/* 0x1264B4: bfset (0x30EE,A5){bit:1} -- bfset with Dn bit = 0 sets */
static void bfset_bit(const f3mem_t *m, unsigned bit)
{
    uint32_t a = F3_A5 + 0x30EEu + bit / 8u;
    wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (bit % 8u))));
}

/* ROM 0x12649C: steering tick; on +0x06 expiry set A5+0x30EE{0xC} and hand
 * over to 0x1264CA with a 0x3C-frame timer. */
int Obj_12649C_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x12649C */
    ObjHelper_11B2CA(m, obj);                          /* 0x1264A2 */
    ObjHelper_11B262(m, obj);                          /* 0x1264A8 */

    /* 0x1264AE: subq.w #1 -> bgt 0x126494 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0012649Cu);             /* 0x126494 */
        return 0;
    }

    bfset_bit(m, 0x0Cu);                               /* 0x1264B4 */
    wr16(m, obj + 0x06u, 0x003Cu);                     /* 0x1264BA */
    /* 0x1264C0 beq -> 0x1264E2: move.w only leaves Z set for a stored zero,
     * so the ROM takes the fall-through to 0x1264C2. */
    wr32(m, obj + 0x00u, 0x001264CAu);                 /* 0x1264C2 */
    return 0;
}
