#include "obj_108D8E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x108D8E: group flag / dying gate, then a respawn counter that spawns a
 * 0x108DFC object; +0x4E of the new object mirrors +0x4E of the parent. */
int Obj_108D8E_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x04u)          /* btst.b #5 */
        return 0;                                  /* Obj_StartDying (0xCEE74) */
    if (rd8(m, obj + 0x22u) == 0u)
        return 0;                                  /* Obj_StartDying (0xCEE74) */

    if (rd8(m, obj + 0x21u) != 0u) {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t != 0u)
            return 0;
        wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
        wr8(m, obj + 0x23u, 0x14u);
        {
            uint32_t a1 = Obj_Spawn(m, obj, 0x00108DFCu, 5u, 2u, 0u);
            if (a1 == 0xFFFFFFFFu)
                return 0;                          /* 0x108DF6 */
            wr8(m, a1 + 0x4Eu, 0u);
            if (rd8(m, obj + 0x21u) == 0u)
                wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
        }
    }
    return 0;
}
