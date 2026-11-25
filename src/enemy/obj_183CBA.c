#include "obj_183CBA.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x183CBA: spawn-wait start. Arms the +0x4C timer from the global at
 * A5+0x1D28, gives up (0x17C78E, free) when that global is zero, places the
 * object from its +0x0C record (0x17BD68), leaves the 0x1849F8 region the same
 * way, and otherwise hands the object to the 0x183CF6 wait body: the first
 * tick's frame count travels in +0x64. */

/* ROM 0x17C78E: off-screen / dead, free the object */
static void lab_17C78E(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
}

/* cmp2.w (0x1849F8) / (0x1849FC): inside the on-screen box (bcs -> 0x17C78E) */
static int in_bounds(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu))
        return 0;
    if (y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu))
        return 0;
    return 1;
}

int Obj_183CBA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x183CBA: move.l (0x1D28,A5),(0x4C,A0) ; ble.w 0x17C78E */
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
    if ((int32_t)rd32(m, obj + 0x4Cu) <= 0) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* ROM 0x183CC4: bsr.w 0x17BD68 */
    Obj_InheritOffsetPos(m, obj);

    /* ROM 0x183CC8..0x183CE4: cmp2.w (0x1849F8)/(0x1849FC) ; bcs.w 0x17C78E */
    if (!in_bounds(m, obj)) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* ROM 0x183CE8: move.w (0xE,A0),(0x64,A0) -- the wait body's frame count */
    wr16(m, obj + 0x64u, rd16(m, obj + 0x0Eu));

    /* ROM 0x183CEE: move.l #0x183CF6,(A0) */
    wr32(m, obj + 0x00u, 0x00183CF6u);
    return 0;
}
