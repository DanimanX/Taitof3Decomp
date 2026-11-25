#include "obj_17D912.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_anim.h"
#include "obj_E8B72.h"

/* ROM 0x17D912: first half of the 17D912 pair -- the cluster shot that carries a
 * 0x17BDA2 packed offset instead of the parent-relative 0x17BD68 one. The
 * record at A5+0x1D28 is required (+0x4C); 0x17BDA2 folds it into
 * +0x16/+0x1A/+0x50/+0x54, the object leaves the 0x1849F8/0x1849FC play area by
 * freeing itself, otherwise one 0x17DA00 child is spawned into group 11
 * (bullets) and the tick continues at 0x17D972 (stat record 0x17DAB4 and frame
 * list), then parks at 0x17C144. Verified bit-exact against 5 MAME ticks. */

/* ROM 0x17C78E: free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* ROM 0x17D972: sound 0x11, seed the stat record 0x17DAB4, load the frame
 * list index at 0x17DACA into +0x40/+0x3A and run its INIT half, then arm the
 * 0x17DB2C record and park at 0x17C144 (+0x00 := 0x17C14C). */
static int lab_17D972(const f3mem_t *m, uint32_t obj)
{
    Sound_PlayHooked(m, 0x0011u);
    Obj_184888(m, obj, 0x0017DAB4u);

    uint32_t a1 = 0x0017DACAu;
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0w);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, d0w) < 0)
        return -1;

    wr32(m, obj + 0x44u, 0x0017DB2Cu);
    wr16(m, obj + 0x48u, rd16(m, 0x0017DB30u));
    wr16(m, obj + 0x4Au, 0u);
    wr32(m, obj + 0x0Cu, 0x0017D9CEu);
    wr32(m, obj + 0x00u, 0x0017C14Cu);       /* 0x17C144 */
    return 0;
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

    /* 0x17D95C: move.w #0,(0x6,A0) ; 0x17D962 beq.b 0x17D972. move.w #0 sets
     * Z=1, so the branch is always taken and the arm of 0x17D964
     * (+0x00 := 0x17D96C) never runs: fall into 0x17D972 directly. */
    wr16(m, obj + 0x06u, 0x0000u);
    return lab_17D972(m, obj);
}

