/* ROM 0x18A474: clears the 0x611000 block, frees the A5+0x4B74 object, spawns
 * the 0x14D75A actor, then arms the 8-frame state 0x18A4CA. */
#include "obj_18A474.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

#define A5_B74   (F3_A5 + 0x4B74u)

/* The fall-through behind the beq at 0x18A4C0 (0x18A4D0 -> 0x189A8E) is dead:
 * it tests the Z left by `move.w #8,(A0+6)`, always 0. */

/* ROM 0x18A474 */
int Obj_18A474_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A474: subq.w #1,(A0+6) ; bgt 0x18A46C (re-arm self) */
    if ((int16_t)(rd16(m, obj + 0x06u) - 1u) > 0) {
        wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
        wr32(m, obj, 0x0018A474u);
        return 0;
    }
    wr16(m, obj + 0x06u, 0);

    /* 0x18A47A: pea (0x611000).l ; move.l #0x81648,-(SP) ; jsr 0x54AC */
    Video_ClearRect(m, 0x00081648u, 0x00611000u);

    /* 0x18A490: move.l (0x4B74,A5),-(SP) ; jsr 0x105EC */
    Obj_Free(m, rd32(m, A5_B74));

    /* 0x18A49E: pea (0x0).w ; move.w #2,-(SP) ; move.w #3,-(SP) ;
     *           move.l #0x14D75A,-(SP) ; jsr 0x1057C */
    (void)Obj_Spawn(m, obj, 0x0014D75Au, 3u, 2u, 0x00000000u);

    /* 0x18A4BA: move.w #8,(A0+6) ; move.l #0x18A4CA,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj, 0x0018A4CAu);
    return 0;
}
