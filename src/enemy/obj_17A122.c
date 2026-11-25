#include "obj_17A122.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17A122: the first tick of the 0x17A1D4 entry object (the one spawned
 * from the boss family's child lists, e.g. by 0x1865A0). While the shared slot
 * counter A5+0x4B24 is above 0x1E it frees itself (0x179D18); otherwise it takes
 * a slot, parks on the parent-relative offset in +0x0C (0x179AE4), installs the
 * 0x17A1D4 movement script, starts frame list 0x17A190 (index 0, through its
 * init 0x179550) and hands the slot to the generic walker 0x179C6E with a random
 * heading byte and turn timer. */

/* ROM 0x179D18: drop the slot counter and free the object */
static void sub_179D18(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) - 1u));
    Obj_Free(m, obj);                           /* 0x0105EC */
}

/* ROM 0x179AE4: (dx.w, dy.w) at +0x0C rides on the parent's +0x50/+0x54 */
static void sub_179AE4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    int16_t d0 = (int16_t)rd16(m, obj + 0x0Cu);
    int16_t d1 = (int16_t)rd16(m, obj + 0x0Eu);
    wr16(m, obj + 0x50u, (uint16_t)(d0 + rd16(m, a4 + 0x50u)));
    wr16(m, obj + 0x54u, (uint16_t)(d1 + rd16(m, a4 + 0x54u)));
}

int Obj_17A122_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A122: cmpi.w #0x1E,(0x4B24,A5) ; bgt.w 0x179D18 */
    if ((int16_t)rd16(m, F3_A5 + 0x4B24u) > 0x1E) {
        sub_179D18(m, obj);
        return 0;
    }
    wr16(m, F3_A5 + 0x4B24u, (uint16_t)(rd16(m, F3_A5 + 0x4B24u) + 1u));  /* 0x17A12C */

    sub_179AE4(m, obj);                         /* 0x17A130 */

    /* 0x17A134: the movement script 0x17A1D4 -- {0x178C06, -1, 0x80} then
     * {0, 0x0C6D, 0x001E4B24}: the second record's frames/arg fields hold live
     * code, so its loop pass sends the walk through 0x179BDA2 / 0x17A26A. */
    {
        uint32_t a1 = 0x0017A1D4u;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }
    wr16(m, obj + 0x4Au, 0u);                   /* 0x17A144 */

    /* 0x17A148: frame list 0x17A190 -- {index 0, dur 2, spr 0x0004FA38, 0, 0}
     * so +0x40 = 0 and +0x3A points at the record body */
    {
        uint32_t a1 = 0x0017A190u;
        uint16_t d0 = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, d0);
        wr32(m, obj + 0x3Au, a1);
        /* 0x17A158: movea.l (0x0,A1,D0w*8),A1 with A1 = 0x179A5C ; jsr (A1) */
        if (Obj_AnimInit(m, obj, d0) < 0)
            return -1;
    }

    wr16(m, obj + 0x1Eu, 0u);                   /* 0x17A164 */
    wr16(m, obj + 0x1Cu, 0u);                   /* 0x17A168 */

    /* 0x17A16C: moveq #0x10,D0 ; jsr 0x1794A4 ; move.b D0b,(0x20,A0) */
    wr8(m, obj + 0x20u, (uint8_t)Obj_RandRange(m, 0x10u));

    /* 0x17A178: move.l #0x100,D0 ; jsr 0x1794A4 ; addi.w #0x200,D0 ;
     * move.w D0w,(0x42,A0) */
    wr16(m, obj + 0x42u, (uint16_t)(Obj_RandRange(m, 0x0100u) + 0x200u));

    wr32(m, obj + 0x00u, 0x00179C6Eu);          /* 0x17A18C bra.w 0x179C66 */
    return 0;
}
