#include "obj_126AE2.h"
#include "obj_11B262.h"

/* ROM 0x126AE2: steering tick. Countdown expiry publishes flag
 * A5+0x30EE{0x10:1} and hands over to 0x126B10 with a 0x3C-frame timer. */
int Obj_126AE2_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126AE2 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126AE8 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126AEE jsr 0x11B262 */

    /* 0x126AF4: subq.w #1 -> bgt 0x126ADA tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126AE2u);         /* 0x126ADA */
        return 0;
    }
    /* 0x126AFA: bfset (0x30EE,A5){0x10:1} (bit 7 of the byte at A5+0x30F0) */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x10u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x10u % 8u))));
    }
    wr16(m, obj + 0x06u, 0x003Cu);                 /* 0x126B00 */
    /* 0x126B06: beq 0x126B28 tests the Z set by move.w of 0x3C: not taken */
    wr32(m, obj + 0x00u, 0x00126B10u);             /* 0x126B08 */
    return 0;
}
