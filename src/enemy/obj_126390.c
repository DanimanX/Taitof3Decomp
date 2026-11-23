#include "obj_126390.h"
#include "obj_11B262.h"

/* ROM 0x126390: steering tick (0x1261AA family). Follow the leader and ease
 * the turn, then count +0x06 down; on expiry set A5+0x30EE{0x7}, re-arm 0xFA
 * and hand over to 0x1263BE. 0x1263B4 is a `beq` after `move.w #0xFA,(0x6,A0)`
 * whose flag producer the bundle truncates; the golden pairs show the branch
 * NOT taken, so it always falls into 0x1263B6. */
int Obj_126390_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126390 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126396 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12639C jsr 0x11B262 */

    /* 0x1263A2: subq.w #1 -> bgt 0x126388 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126390u);         /* 0x126388 */
        return 0;
    }

    /* 0x1263A8: bfset (0x30EE,A5){0x7:0x1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x07u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x07u % 8u))));
    }
    wr16(m, obj + 0x06u, 0x00FAu);                 /* 0x1263AE */
    wr32(m, obj + 0x00u, 0x001263BEu);             /* 0x1263B6 */
    return 0;
}
