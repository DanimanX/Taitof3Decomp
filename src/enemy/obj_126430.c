#include "obj_126430.h"
#include "obj_11B262.h"

/* ROM 0x126430: steering tick (tail of the 0x1261AA family). Follows the
 * leader and eases the turn, then counts +0x06 down; on expiry set
 * A5+0x30EE{0x6}, re-arm 0xB4 and hand over to 0x12645E. 0x126454 is a `beq`
 * after `move.w #0xB4,(0x6,A0)` whose flag producer the bundle truncates; the
 * golden pairs show the branch NOT taken, so it always falls into 0x126456. */
int Obj_126430_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126430 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126436 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x12643C jsr 0x11B262 */

    /* 0x126442: subq.w #1 -> bgt 0x126428 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126430u);         /* 0x126428 */
        return 0;
    }

    /* 0x126448: bfset (0x30EE,A5){0x6:0x1} */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x06u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x06u % 8u))));
    }
    wr16(m, obj + 0x06u, 0x00B4u);                 /* 0x12644E */
    wr32(m, obj + 0x00u, 0x0012645Eu);             /* 0x126456 */
    return 0;
}
