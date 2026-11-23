#include "obj_12D39C.h"
#include "../objects/obj_pool.h"

/* ROM 0x12D39C: drifts the view by (+0x222,+0x166) per frame, then every 0x5A
 * frames spawns two 0x6E-tick objects (0x13029E in group 3, 0x1302F8 in group
 * 3 next to it) and moves on to 0x12D3FA. */
int Obj_12D39C_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) + 0x0222u));
    wr16(m, F3_A5 + 0x32A4u, (uint16_t)(rd16(m, F3_A5 + 0x32A4u) + 0x0166u));

    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        /* 0x12D394 */
        wr32(m, obj + 0x00u, 0x0012D39Cu);
        return 0;
    }

    /* 0x12D3AE / 0x12D3CC: arg 0x6E into group 3 (2 groups) */
    (void)Obj_Spawn(m, obj, 0x0013029Eu, 3u, 2u, 0x0000006Eu);
    (void)Obj_Spawn(m, obj, 0x001302F8u, 3u, 2u, 0x0000006Eu);

    wr16(m, obj + 0x06u, 0x005Au);                       /* 0x12D3EA */
    /* 0x12D3F0 beq: Z was cleared by the move.w above, never taken */
    wr32(m, obj + 0x00u, 0x0012D3FAu);                   /* 0x12D3F2 */
    return 0;
}
