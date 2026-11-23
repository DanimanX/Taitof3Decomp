#include "obj_1268D4.h"
#include "obj_11B262.h"

/* ROM 0x1268D4: steering tick. The prologue is the 0x11B288/0x11B2CA/0x11B262
 * trio: turn +0x20 toward the leader (0x006422 heading, clamp 2/frame), ease
 * +0x56 toward +0x54 and mirror it to +0x42, then follow the leader (A5+0x30D2
 * -> +0x14, A5+0x30D6 -> +0x18, 0x0CBE12 with (0x20,A0)>>8, and publish the
 * moved +0x14/+0x18 back). On +0x06 expiry bit A5+0x30EE{0xC} is set and a
 * fresh 0x1E timer installed (Z clear, so `beq 0x12691A` is not taken): the
 * tail installs 0x126902, whose own expiry likewise falls through to 0x12691A
 * and hands over to 0x126936 after a 0x3E-frame pause. */
int Obj_1268D4_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1268D4 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1268DA jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1268E0 jsr 0x11B262 */

    /* 0x1268E6: subq.w #1 -> bgt 0x1268CC tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001268D4u);         /* 0x1268CC */
        return 0;
    }

    /* 0x1268EC: bfset (0x30EE,A5){0xC:1} -> bit 3 of the long word +0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Cu / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Cu % 8u))));
    }
    wr16(m, obj + 0x06u, 0x001Eu);                 /* 0x1268F2 */
    /* 0x1268F8: beq 0x12691A -- Z clear after the 0x1E store: not taken */
    wr32(m, obj + 0x00u, 0x00126902u);             /* 0x1268FA */
    return 0;
}
