#include "obj_17DDD0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x17DDD0: first half of the 17DDD0 pair, the second cluster shot of the
 * family. Places the object from its polar record (+0x0C), folds the parent's
 * position in, then spawns one 0x17C856 child into group 11 (bullets) and parks
 * in the 0x17DE20 one-frame wait. Returns -1 only if Obj_PolarPlace fails
 * (it cannot, in this tree). Observed as 10/10 bit-exact ticks, re-checked
 * 2026-11-25. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

int Obj_17DDD0_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t x, y;

    /* 0x17DDD0 bsr.w 0x17BDA2: polar record +0x0C -> +0x16/+0x1A/+0x20,
     * then +0x50/+0x54 := parent + offset */
    if (Obj_PolarPlace(m, obj) != 0)
        return -1;

    /* 0x17DDD4..0x17DDF0: cmp2.w (0x1849F8)/(0x1849FC) ; bcs -> 0x17C78E */
    x = (int16_t)rd16(m, obj + 0x50u);
    y = (int16_t)rd16(m, obj + 0x54u);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu) ||
        y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17DDF4: pea #0 ; move.w #1,-(SP) ; move.w #0xB,-(SP) ;
     * move.l #0x17C856,-(SP) ; jsr 0x1057C. 0x17DE10 move.w #1,(0x6,A0), then
     * 0x17DE16 beq.b tests the spawn result in D0 (the move keeps its flags);
     * a full pool returns 0xFFFFFFFF and falls through to 0x17DE18, which
     * installs the same 0x17DE20 as the taken branch at 0x17DE26. */
    uint32_t child = Obj_Spawn(m, obj, 0x0017C856u, 0x000Bu, 0x0001u, 0u);
    wr16(m, obj + 0x06u, 0x0001u);
    if (child == 0u)
        wr32(m, obj + 0x00u, 0x0017DE20u); /* 0x17DE18 */
    wr32(m, obj + 0x00u, 0x0017DE20u);     /* 0x17DE26 */
    return 0;
}
