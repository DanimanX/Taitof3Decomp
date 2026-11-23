#include "obj_126A7A.h"
#include "obj_11B262.h"

/* ROM 0x126A7A: steering tick. Countdown expiry retargets the turn toward
 * +0x54 = 8 at (+0x50,+0x52) = (0xA0,0x78) with a 0x78-frame timer and hands
 * over to 0x126AB4. */
int Obj_126A7A_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126A7A jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126A80 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126A86 jsr 0x11B262 */

    /* 0x126A8C: subq.w #1 -> bgt 0x126A72 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126A7Au);         /* 0x126A72 */
        return 0;
    }
    /* 0x126A92 */
    wr8(m, obj + 0x54u, 0x08u);
    wr16(m, obj + 0x50u, 0x00A0u);                 /* 0x126A98 */
    wr16(m, obj + 0x52u, 0x0078u);                 /* 0x126A9E */
    wr16(m, obj + 0x06u, 0x0078u);                 /* 0x126AA4 */
    /* 0x126AAA: beq 0x126ACC tests the Z set by move.w of 0x78: not taken */
    wr32(m, obj + 0x00u, 0x00126AB4u);             /* 0x126AAC */
    return 0;
}
