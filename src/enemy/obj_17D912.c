#include "obj_17D912.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x17D912: first half of the 17D912 pair -- the cluster shot that carries a
 * 0x17BDA2 packed offset instead of the parent-relative 0x17BD68 one. The
 * record at A5+0x1D28 is required (+0x4C); 0x17BDA2 folds it into
 * +0x16/+0x1A/+0x50/+0x54, the object leaves the 0x1849F8/0x1849FC play area by
 * freeing itself, otherwise one 0x17DA00 child is spawned into group 11
 * (bullets) and the object parks in the 0x17D96C hold before the 0x17DAB4 stat
 * record and frame list are loaded.
 *
 * Not verifiable against a capture yet: work/ram/obj_17D912_pairs.bin holds 5
 * ticks but no ROM image in the test harness, so the 0x17BDA2 table lookups
 * read as zero there. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

int Obj_17D912_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D912: move.l (0x1D28,A5),(0x4C,A0) ; beq.w 0x17C78E */
    uint32_t rec = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, obj + 0x4Cu, rec);
    if (rec == 0u) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* 0x17D91C: bsr.w 0x17BDA2 -- packed offset at +0x0C into +0x16/+0x1A and
     * the position at parent +0x50/+0x54 */
    (void)Obj_PolarPlace(m, obj);

    /* 0x17D920..0x17D93C: cmp2.w (0x1849F8) -0x10 <= +0x50 <= 0x14F and
     * cmp2.w (0x1849FC) -0x10 <= +0x54 <= 0xF7 ; bcs -> 0x17C78E (free) */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t y = (int16_t)rd16(m, obj + 0x54u);
        if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu) ||
            y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
            lab_17C78E(m, obj);
            return 0;
        }
    }

    /* 0x17D940: pea #0 ; move.w #1,-(SP) ; move.w #0xB,-(SP) ;
     * move.l #0x17DA00,-(SP) ; jsr 0x1057C (spawn parent, group 11, 1 group) */
    (void)Obj_Spawn(m, obj, 0x0017DA00u, 0x000Bu, 0x0001u, 0u);

    /* 0x17D95C: move.w #0,(0x6,A0) ; 0x17D962 beq.b tests the flags the move
     * left, i.e. Z=1 always -> 0x17D972. 0x17D964 (+0x00 := 0x17D96C) is
     * skipped, the tick is always armed at 0x17D96C. */
    wr16(m, obj + 0x06u, 0x0000u);
    wr32(m, obj + 0x00u, 0x0017D96Cu);
    return 0;
}

