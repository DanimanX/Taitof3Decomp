#include "obj_14C9FE.h"
#include "../objects/obj_pool.h"

/* ROM 0x14C9FE: muzzle-flash emitter. While +0x06 counts down it only
 * re-publishes itself (0x14C9F6); when it expires it spawns one 0x14CC12
 * (group 0, one slot) and hands its slot to that routine (0x14CA04). */
int Obj_14C9FE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x14C9FE: subq.w #1,(0x6,A0) ; bgt 0x14C9F6 */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0014C9FEu);                 /* 0x14C9F6 */
        return 0;
    }

    /* 0x14CA04: arg = 0, group 0, 1 slot, routine 0x14CC12 */
    (void)Obj_Spawn(m, obj, 0x0014CC12u, 0u, 1u, 0x00000000u);
    wr32(m, obj + 0x00u, 0x0014CAFAu);                     /* 0x14CAF2 */
    return 0;
}
