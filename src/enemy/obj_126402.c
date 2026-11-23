#include "obj_126402.h"
#include "obj_11B262.h"

/* ROM 0x126402: steering tick (head of the 0x1261AA family). Runs the three
 * tracker helpers, counts +0x06 down and, on expiry, sets A5+0x30EE{0x2},
 * re-arms 0x64 and hands over to 0x126430.
 *
 * Raw ROM at 0x126402 is
 *   4EB9 0011B288 4EB9 0011B2CA 4EB9 0011B262 5368 0006 6EE0
 *   EEED 0081 30EE 317C 0064 0006 6720 20BC 00126430 4E75
 * so the flow is: jsr x3, subq.w #1,(0x6,A0), bgt.b 0x126402, then
 *   0x12641A bfset (0x30EE,A5){0x2:0x1}       eeed 0081 30ee
 *   0x126420 move.w #0x64,(0x6,A0)            317c 0064 0006
 *   0x126426 beq.b 0x126448                   6720
 * The bundle dropped the 0x126426 branch; decoded from ROM it is a plain
 * `beq.b`, so it consumes the Z flag of the `move.w` immediately before it,
 * which moves 0x64 (non-zero) and always clears Z. The branch is therefore
 * never taken and control always falls into 0x126428 -> 0x126430. */
int Obj_126402_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                   /* 0x126402 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                   /* 0x126408 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                   /* 0x12640E jsr 0x11B262 */

    /* 0x126414: subq.w #1 -> bgt 0x126402 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126402u);      /* 0x126402 */
        return 0;
    }

    /* 0x12641A: bfset (0x30EE,A5){0x2:0x1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x02u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x02u % 8u))));
    }
    wr16(m, obj + 0x06u, 0x0064u);              /* 0x126420 */
    /* 0x126426 beq.b: always false (see above) */
    wr32(m, obj + 0x00u, 0x00126430u);          /* 0x126428 */
    return 0;
}
