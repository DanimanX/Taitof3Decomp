#include "obj_12D17E.h"
#include "../objects/obj_pool.h"

/* ROM 0x12D17E: the mid-screen orb/laser trail shrinks (A5+0x32A6) and the
 * player's shot speed (A5+0x32A2) ramps by 0xB00 a frame; when the speed
 * reaches 0x400 one 0x1302F8 object is spawned and the timer/ramp are reset. */
int Obj_12D17E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x32A6u) > 0)
        wr16(m, F3_A5 + 0x32A6u, (uint16_t)(rd16(m, F3_A5 + 0x32A6u) - 2u));
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) + 0x0B00u));
    if (rd16(m, F3_A5 + 0x32A2u) != 0x0400u) {
        wr32(m, obj, 0x0012D17Eu);          /* LAB_0012d176 */
        return 0;
    }

    uint32_t child = Obj_Spawn(m, obj, 0x001302F8u, 3u, 2u, 0x28u);
    wr16(m, obj + 0x06u, 0x1Eu);

    /* TST.W of the child pointer only clears Z when the spawn failed, so a
     * successful spawn always takes the plain re-arm at 0x12D1BE. */
    if (child == 0u) {
        wr16(m, F3_A5 + 0x32A4u, 0u);
        wr16(m, F3_A5 + 0x32A8u, 3u);
        wr32(m, obj, 0x0012D1E0u);          /* LAB_0012d1d8 */
        return 0;
    }
    wr32(m, obj, 0x0012D1C6u);              /* LAB_0012d1be */
    return 0;
}
