#include "obj_126AB4.h"
#include "obj_11B262.h"

/* ROM 0x126AB4: steering tick. Countdown expiry straightens the turn
 * (+0x54 = 0) with a 0x3C-frame timer and hands over to 0x126AE2. */
int Obj_126AB4_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126AB4 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126ABA jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126AC0 jsr 0x11B262 */

    /* 0x126AC6: subq.w #1 -> bgt 0x126AAC tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126AB4u);         /* 0x126AAC */
        return 0;
    }
    /* 0x126ACC */
    wr8(m, obj + 0x54u, 0x00u);
    wr16(m, obj + 0x06u, 0x003Cu);                 /* 0x126AD2 */
    /* 0x126AD8: beq 0x126AFA tests the Z set by move.w of 0x3C: not taken */
    wr32(m, obj + 0x00u, 0x00126AE2u);             /* 0x126ADA */
    return 0;
}
