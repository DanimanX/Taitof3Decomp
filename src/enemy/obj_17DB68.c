#include "obj_17DB68.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x17DB68: first half of the 17DB68 pair -- a cluster shot. The
 * parent-relative offset is folded into +0x50/+0x54, then one 0x17C856 child
 * is spawned into group 11 (bullets); the object parks in the 0x17DBB8
 * one-frame wait before the 0x17DD50 stat record and frame list are loaded.
 *
 * Observed as 19/19 bit-exact ticks with the spawn and both +0x06/+0x00 stores
 * applied (the capture's post state shows the 0x17C856 child in group 11, the
 * +0x06 counter at 1 and +0x00 = 0x17DBB8), re-checked 2026-11-25. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

int Obj_17DB68_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t x, y;

    /* 0x17DB68 bsr.w 0x17BD68: +0x16/+0x1A := +0x0C/+0x0E, +0x50/+0x54 := parent */
    Obj_InheritOffsetPos(m, obj);

    /* 0x17DB6C..0x17DB88: cmp2.w (0x1849F8) -0x10 <= +0x50 <= 0x14F and
     * cmp2.w (0x1849FC) -0x10 <= +0x54 <= 0xF7 ; bcs -> 0x17C78E (free) */
    x = (int16_t)rd16(m, obj + 0x50u);
    y = (int16_t)rd16(m, obj + 0x54u);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu) ||
        y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17DB8C: pea #0 ; move.w #1,-(SP) ; move.w #0xB,-(SP) ;
     * move.l #0x17C856,-(SP) ; jsr 0x1057C (spawn parent, group 11, 1 group).
     * 0x17DBA8: move.w #1,(0x6,A0) ; 0x17DBAE beq.b tests the flags the move
     * left (it does not set them), i.e. the spawn result in D0: 0xFFFFFFFF on
     * a full pool takes the branch to 0x17DBBE, whose next instruction is
     * 0x17DBB0 itself -- both paths arm 0x17DBB8. */
    uint32_t child = Obj_Spawn(m, obj, 0x0017C856u, 0x000Bu, 0x0001u, 0u);
    wr16(m, obj + 0x06u, 0x0001u);
    if (child != 0u)
        wr32(m, obj + 0x00u, 0x0017DBB8u); /* 0x17DBB0 */
    wr32(m, obj + 0x00u, 0x0017DBB8u);
    return 0;
}
