#include "obj_126936.h"
#include "obj_11B262.h"

/* ROM 0x126936: steering phase tick. Runs the 0x11B288 / 0x11B2CA / 0x11B262
 * tracker trio and counts down +0x06. On expiry, branches on A5+0x30DA <= 0x3E8
 * to either clear A5+0x30EE and transition to 0x126976, or set up the next
 * steering target (0x40, 0x40) with +0x54 = 8 and transition to 0x126710. */
int Obj_126936_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126936 jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x12693C jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126942 jsr 0x11B262 */

    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126936u);         /* 0x12692E */
        return 0;
    }

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) <= 0x03E8) {
        wr32(m, F3_A5 + 0x30EEu, 0u);              /* 0x12695C */
        wr8(m, obj + 0x54u, 0u);                   /* 0x126960 */
        wr16(m, obj + 0x06u, 0x0078u);             /* 0x126966 */
        wr32(m, obj + 0x00u, 0x00126976u);         /* 0x12696E */
        return 0;
    }

    wr8(m, obj + 0x54u, 0x08u);                    /* 0x1266EE */
    wr16(m, obj + 0x50u, 0x0040u);                 /* 0x1266F4 */
    wr16(m, obj + 0x52u, 0x0040u);                 /* 0x1266FA */
    wr16(m, obj + 0x06u, 0x0078u);                 /* 0x126700 */
    wr32(m, obj + 0x00u, 0x00126710u);             /* 0x126708 */
    return 0;
}
