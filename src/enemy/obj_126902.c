#include "obj_126902.h"
#include "obj_11B262.h"

/* ROM 0x126902: steering tick. The 0x11B288/0x11B2CA/0x11B262 trio, then the
 * +0x06 countdown: expiry sets A5+0x30EE{0xD} and counts +0x3E down; while it
 * stays positive the tick re-runs 0x1268C4 (0x1E timer, back to 0x1268D4).
 * Otherwise a 0x3C timer hands over to 0x126936, and when the counter above
 * 0x7D0 is still live (bra 0x1266EE falling into 0x12695C) A5+0x30EE is
 * cleared and 0x126976 installed (the 0x12698E sound path is unreachable:
 * `beq` tests the Z flag left clear by the 0x78 store). */
int Obj_126902_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                          /* 0x126902 */
    ObjHelper_11B2CA(m, obj);                          /* 0x126908 */
    ObjHelper_11B262(m, obj);                          /* 0x12690E */

    /* 0x126914: subq.w #1 -> bgt 0x1268FA tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126902u);             /* 0x1268FA */
        return 0;
    }
    /* 0x12691A: bfset (0x30EE,A5){0xD:1} -> bit 5 of A5+0x30EF */
    {
        uint32_t a = F3_A5 + 0x30EEu + 0x0Du / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Du % 8u))));
    }
    /* 0x126920: subq.w #1 -> bgt 0x1268C4 tests the NEW value */
    uint16_t c = (uint16_t)(rd16(m, obj + 0x3Eu) - 1u);
    wr16(m, obj + 0x3Eu, c);
    if ((int16_t)c > 0) {
        wr16(m, obj + 0x06u, 0x001Eu);                 /* 0x1268C4 */
        wr32(m, obj + 0x00u, 0x001268D4u);             /* 0x1268CC */
        return 0;
    }
    wr16(m, obj + 0x06u, 0x003Cu);                     /* 0x126926; Z clear */
    wr32(m, obj + 0x00u, 0x00126936u);                 /* 0x12692E */
    /* 0x12694E: cmpi.w #0x7D0,(0x30DA,A5); ble 0x12695C */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x07D0)
        return 0;

    /* 0x126958: bra.w 0x1266EE -- falls into 0x12695C */
    wr32(m, F3_A5 + 0x30EEu, 0u);                      /* 0x12695C clr.l */
    wr8(m, obj + 0x54u, 0x00u);                        /* 0x126960 */
    wr16(m, obj + 0x06u, 0x0078u);                     /* 0x126966; Z clear */
    wr32(m, obj + 0x00u, 0x00126976u);                 /* 0x12696E */
    return 0;
}
