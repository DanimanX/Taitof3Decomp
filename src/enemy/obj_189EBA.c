#include "obj_189EBA.h"
#include "../objects/obj_pool.h"

/* The ROM 0x189EBA tick is the 8-frame colour hold of the 0x1899B0 family. */

/* ROM 0x189EB2: re-arm the tick vector with 0x189EBA. */
static void arm_self(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x00189EBAu);
}

/* ROM 0x189EBA: count +0x06 down and re-arm while it stays positive. On expiry
 * it reloads the sprite of the object at A5+0x4B74 with 0x28740, restarts its
 * own countdown at 8 and re-arms 0x189EDC -- the colour-merge step that then
 * hands the object back to 0x1899B0 (the 0x189A8E tail lives in that tick).
 *
 * The beq at 0x189ED2 tests the Z of `move.w #8,(0x6,A0)`, which never sets it,
 * so the branch to 0x189A8E behind it is dead and the re-arm always lands. */
int Obj_189EBA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x189EBA: subq.w #1,(0x6,A0) ; bgt.s 0x189EB2 */
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        arm_self(m, obj);
        return 0;
    }

    /* 0x189EC0: movea.l (0x4B74,A5),A1 ; move.l #0x28740,(0x10,A1) */
    wr32(m, rd32(m, F3_A5 + 0x4B74u) + 0x10u, 0x00028740u);

    /* 0x189ECC: move.w #8,(0x6,A0) ; move.l #0x189EDC,(A0) */
    wr16(m, obj + 0x06u, 0x0008u);
    wr32(m, obj + 0x00u, 0x00189EDCu);
    return 0;
}
