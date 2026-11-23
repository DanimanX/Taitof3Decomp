#include "obj_126844.h"
#include "obj_11B262.h"

/* ROM 0x126844: steering tick. The 0x11B288/0x11B2CA/0x11B262 trio; every
 * 0x40th frame of the +0x06 counter (low six bits == 0) sets A5+0x30EE{0xF}.
 * On countdown expiry a 0x14 timer hands over to 0x12687E. */
int Obj_126844_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126844 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x12684A jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126850 jsr 0x11B262 */

    /* 0x126856: D0w = (0x6,A0) & 0x3F; bne 0x126868 */
    if ((rd16(m, obj + 0x06u) & 0x003Fu) == 0u) {
        /* 0x126862: bfset (0x30EE,A5){0xF:1} -> bit 7 of A5+0x30EF */
        uint32_t a = F3_A5 + 0x30EEu + 0x0Fu / 8u;
        wr8(m, a, (uint8_t)(rd8(m, a) | (uint8_t)(0x80u >> (0x0Fu % 8u))));
    }
    /* 0x126868: subq.w #1 -> bgt 0x12683C tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126844u);         /* 0x12683C */
        return 0;
    }
    wr16(m, obj + 0x06u, 0x0014u);                 /* 0x12686E; Z clear */
    wr32(m, obj + 0x00u, 0x0012687Eu);             /* 0x126876 */
    return 0;
}
