#include "obj_17D136.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x17D136: spawn-and-wait tail. It places the object from its polar
 * record (0x17BDA2), gives up (0x17C78E, free) once it leaves the 0x1849F8
 * region, spawns the 0x17C856 child (group 0x0B, one group, arg 0), arms the
 * +0x06 wait and parks itself on 0x17D186.
 *
 * The 0x17D186 wait body then plays sound 0x2A, seeds the stats from 0x17D1E0,
 * starts animation index 0x000A (record 0x17D1F6) and takes the 0x17D2B8
 * movement script. That body is NOT modelled here: every captured tick of
 * 0x17D136 lands on the first frame, which always ends at the 0x17D17E latch,
 * so the tail is left for the routine that owns it. */

/* ROM 0x17C78E: off-screen, free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

int Obj_17D136_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17D136: bsr.w 0x17BDA2 */
    if (Obj_PolarPlace(m, obj) < 0)
        return -1;

    /* 0x17D13A..0x17D156: cmp2.w (0x1849F8) then (0x1849FC) ; bcs 0x17C78E */
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

    /* 0x17D15A..0x17D172: pea (0x0).w ; move.w #1,-(A7) ; move.w #0xB,-(A7) ;
     * move.l #0x17C856,-(A7) ; jsr 0x1057C
     * -- Obj_Spawn(parent, fn, group, n, arg) */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x0017C856u, 11u, 1u, 0u);

        /* 0x17D176: move.w #1,(0x6,A0) ; beq.b 0x17D18C.
         * The Z flag tested here is whatever 0x1057C left behind (its movem
         * restore sets none of it from the result), so the 0x17D18C arm is not
         * a "spawn failed" test and the wait path is taken unconditionally. */
        wr16(m, obj + 0x06u, 1u);
        (void)child;
    }
    wr32(m, obj + 0x00u, 0x0017D186u);                 /* 0x17D17E */
    return 0;
}
