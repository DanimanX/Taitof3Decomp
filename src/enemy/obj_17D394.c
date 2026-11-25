#include "obj_17D394.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x17D394: spawn tail. It places the object from the parent's position
 * plus its own offsets (0x17BD68), gives up (0x17C78E, free) once +0x50/+0x54
 * leave the 0x1849F8 region, spawns the 0x17CCBE child (group 0x0B, one
 * group, arg 0x00140000), arms a 0x23-frame wait in +0x06 and parks itself on
 * 0x17D3E6. The +0x06 arm clears Z, so the wait arm is the one the ROM takes.
 *
 * 0x17D3E6's expiry body (0x17D3EC onward: parent follow, sound, stats, the
 * 0x17D4E0 script and the 0x17C240 hand-over) belongs to that routine, not
 * this one, and is not modelled here. */

/* ROM 0x17C78E: off-screen, free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

int Obj_17D394_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D394: bsr.w 0x17BD68 */
    Obj_InheritOffsetPos(m, obj);

    /* 0x17D398..0x17D3B4: cmp2.w (0x1849F8) then (0x1849FC) ; bcs 0x17C78E */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        int okx = x > (int16_t)rd16(m, 0x001849F8u) && x < (int16_t)rd16(m, 0x001849FAu);
        int oky = y > (int16_t)rd16(m, 0x001849FCu) && y < (int16_t)rd16(m, 0x001849FEu);
        if (!okx || !oky) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x17D3B8..0x17D3CC: pea (0x140000).l ; move.w #1,-(A7) ;
     * move.w #0xB,-(A7) ; move.l #0x17CCBE,-(A7) ; jsr 0x1057C */
    (void)Obj_Spawn(m, obj, 0x0017CCBEu, 11u, 1u, 0x00140000u);

    /* 0x17D3D6: move.w #0x23,(0x6,A0) ; beq.b 0x17D3EC -- never taken */
    wr16(m, obj + 0x06u, 0x0023u);

    /* 0x17D3DE: move.l #0x17D3E6,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x0017D3E6u);
    return 0;
}
