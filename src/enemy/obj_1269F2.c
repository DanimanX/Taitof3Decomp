#include "obj_1269F2.h"
#include "obj_11B262.h"

/* ROM 0x1269F2: last steering tick. On countdown expiry reload A5+0x30E2
 * from A5+0x1D28, set flag bit A5+0x30EE{0x12}, retarget +0x54 = 3 with a
 * 0x258-frame timer and hand over to 0x126A2A. */
int Obj_1269F2_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x1269F2 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x1269F8 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x1269FE jsr 0x11B262 */

    /* 0x126A04: subq.w #1 -> bgt 0x1269EA tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x001269F2u);         /* 0x1269EA */
        return 0;
    }
    /* 0x126A0A */
    wr32(m, F3_A5 + 0x30E2u, rd32(m, F3_A5 + 0x1D28u));
    {
        /* bfset (0x30EE,A5){0x12:1}: bit 2 of the long word at +0x30EF */
        uint32_t a = F3_A5 + 0x30EEu + 0x12u / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x12u % 8u))));
    }
    wr8(m, obj + 0x54u, 0x03u);                    /* 0x126A16 */
    wr16(m, obj + 0x06u, 0x0258u);                 /* 0x126A1C */
    wr32(m, obj + 0x00u, 0x00126A2Au);             /* 0x126A22 */
    return 0;
}
