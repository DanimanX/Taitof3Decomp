#include "obj_126282.h"
#include "obj_11B262.h"

/* ROM 0x126282: steering tick (0x1261FE family). Follow the leader and ease
 * the turn, then count +0x06 down; on expiry clear A5+0x30EE{0xE}, re-arm 0x14
 * and hand over to 0x1262B0.
 * 0x1262A6 is a `beq` whose flag producer (a `move.w #0x14,(0x6,A0)` bit-list
 * store the bundle prints as `bftst`) is truncated here; the golden pairs keep
 * the +0x06 countdown at 0x12C in every tick, so the branch is NOT taken and
 * the fall-through 0x1262A8 always runs. */
int Obj_126282_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126282 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126288 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12628E jsr 0x11B262 */

    /* 0x126294: subq.w #1 -> bgt 0x12627A tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126282u);         /* 0x12627A */
        return 0;
    }

    /* 0x12629A: bfclr (0x30EE,A5){0xE:1} -> bit 6 of A5+0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Eu / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) & (uint8_t)~(0x80u >> (0x0Eu % 8u))));
    }
    wr16(m, obj + 0x06u, 0x0014u);                 /* 0x1262A0 */
    wr32(m, obj + 0x00u, 0x001262B0u);             /* 0x1262A8 */
    return 0;
}
