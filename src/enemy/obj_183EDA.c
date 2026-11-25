#include "obj_183EDA.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x183EDA: spawn-and-place start. Same head as 0x183CBA -- arm +0x4C from
 * A5+0x1D28, place from the +0x0C record (0x17BD68), free outside the 0x1849F8
 * region -- but it hands over to the 0x183F18 homing body and takes its first
 * frame count from +0x0E. */

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

int Obj_183EDA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x183EDA: move.l (0x1D28,A5),(0x4C,A0) ; ble.w 0x17C78E */
    wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
    if ((int32_t)rd32(m, obj + 0x4Cu) <= 0) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* ROM 0x183EE4: bsr.l 0x17BD68 */
    Obj_InheritOffsetPos(m, obj);

    /* ROM 0x183EEA..0x183F06: cmp2.w (0x1849F8)/(0x1849FC) ; bcs.w 0x17C78E */
    if (!in_bounds(m, obj)) {
        lab_17C78E(m, obj);
        return 0;
    }

    /* ROM 0x183F0A: move.w (0xE,A0),(0x64,A0) */
    wr16(m, obj + 0x64u, rd16(m, obj + 0x0Eu));

    /* ROM 0x183F10: move.l #0x183F18,(A0) */
    wr32(m, obj + 0x00u, 0x00183F18u);
    return 0;
}
