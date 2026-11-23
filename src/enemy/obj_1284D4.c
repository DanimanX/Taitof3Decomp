#include "obj_1284D4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x1284D4: a small sprite fan riding its parent (A1 = (0x8,A0)) at the
 * fixed 16.16 offsets +0x50/+0x52. Each tick it re-sheets the parent's heading
 * byte into its own +0x20, integrates with 0x0CBE12 at speed +0x42, flips the
 * palette +0x1D by 0x2E, indexes the 0x12857C sprite-def table with
 * (heading+4)>>3, draws, re-lists itself as a hazard and raises the speed by
 * 0x20. It keeps ticking while speed < +0x22 and frees itself once the speed
 * catches up, or immediately when the parent has no HP, the global freeze byte
 * A5-0x144B bit 4 is set, or the scroll lock A5+0x1D58 bit 0 is set -- after
 * handing over to the 0x0CD412 script with speed 10.
 *
 * Capture note: the tail branch at 0x12855A targets 0x1284CC, whose ROM body
 * reinstalls 0x1284D4 and returns. The captured fn is therefore unchanged at
 * 0x1284D4 on every pair, so the tail only has to restore the tick it read. */
int Obj_1284D4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);

    if ((int32_t)rd32(m, a1 + 0x24u) <= 0)
        goto stop_12856c;                              /* 0x1284DC */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto stop_12856c;                              /* 0x1284E6 */

    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, a1 + 0x20u) >> 8)); /* 0x1284EA */

    /* 0x1284F4: moveq/move.w/add.w/swap/move.l -- the sum lands in the HIGH
     * word of the 16.16 position */
    {
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 0x14u) + rd16(m, obj + 0x50u));
        wr32(m, obj + 0x14u, (uint32_t)d0 << 16);
        d0 = (uint16_t)(rd16(m, a1 + 0x18u) + rd16(m, obj + 0x52u));
        wr32(m, obj + 0x18u, (uint32_t)d0 << 16);
    }

    /* 0x128514: move.w (0x20,A0),D0w ; jsr 0x0CBE12 (angle = low byte) */
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) & 0xFFu));
    wr8(m, obj + 0x1Du, (uint8_t)(rd8(m, obj + 0x1Du) ^ 0x2Eu)); /* 0x12851E */

    /* 0x128524: addq.b #4 ; lsr.w #3 ; move.l 0x12857C(PC,D0w*4),(0x10,A0) */
    {
        uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) + 0x0004u);
        wr32(m, obj + 0x10u, rd32(m, 0x0012857Cu + 4u * (d0 >> 3)));
    }

    Obj_DrawSprite(m, obj);                            /* 0x128536 -> 0x001282 */
    Coll_AddHazard(m, obj);                            /* 0x12853C -> 0x0CBDAE */
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 0x0020u)); /* 0x128542 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x01u) != 0u)
        goto stop_12856c;                              /* 0x12854E */

    /* 0x128552: move.w (0x42,A0),D0w ; cmp.w (0x22,A0),D0w ; blt 0x1284CC */
    if ((int16_t)rd16(m, obj + 0x42u) >= (int16_t)rd16(m, obj + 0x22u)) {
        Obj_Free(m, obj);                              /* 0x128560 -> 0x0105EC */
        return 0;
    }
    wr32(m, obj + 0x00u, 0x001284D4u);                 /* 0x1284CC reinstall */
    return 0;

stop_12856c:
    /* 0x12856C: speed 10, then jump to script entry 1 (0x0CD3D6, D0 == 1).
     * Jump slot 1 of the table at 0x0CD444 is the empty 0x0CD490 record, so the
     * pointer parks on 0x0CD49A, no sound is queued and the 0x0CD3F6 tail
     * runs; the object keeps its tick. */
    wr16(m, obj + 0x42u, 0x000Au);
    wr32(m, obj + 0x0Cu, 0x000CD49Au);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x00u, 0x000CD412u);
    return 0;
}
