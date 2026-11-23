#include "obj_126158.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../enemy/obj_E8B72.h"
#include "../enemy/obj_11B262.h"

/* ROM 0x126158: the wave's 180-frame stall. +0x06 counts down one frame per
 * tick (0x126158), re-arming 0x126150 while it stays positive. On expiry
 * (0x12615E) it throws a 20-row fan of 0x128924 children into group 5 (the
 * spawn only reads the argument, so a literal 0.0 pair is passed), plays
 * sound 0x15 and hands off to 0x1261AA with heading 0xD0.00 and spin 0x28.00. */
int Obj_126158_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);          /* 0x126158 */
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x00126158u);                       /* 0x126150 */
        return 0;
    }

    /* 0x12615E: move.w #0x13,D1 ; dbra D1 -> 20 children, arg 0 (pea.l #0) */
    for (int i = 0; i < 20; i++) {
        (void)Obj_Spawn(m, obj, 0x00128924u, 5u, 2u, 0x00000000u);
    }

    Sound_PlayHooked(m, 0x0015u);                                /* 0x126182 */
    wr16(m, obj + 0x20u, 0xD000u);                               /* 0x126196 */
    wr16(m, obj + 0x56u, 0x2800u);                               /* 0x12619C */
    wr32(m, obj + 0x00u, 0x001261AAu);                           /* 0x1261A2 */
    return 0;
}
