#include "obj_14CB12.h"
#include "obj_14CB96.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../common/f3mem.h"

/* ROM 0x14CB12: 0x68-frame delay for the wave scroll. While it runs it only
 * re-publishes itself; when it expires it starts three colour ramps (0x203C0
 * -> 0x900, 0x203C0 -> 0x3F80 and 0x14CE0A -> 0) into the palette-slot table,
 * resets the +0x6 timer to 0x26 and hands over to 0x14CB96. */
int Obj_14CB12_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x14CB12 */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        wr32(m, obj + 0x00u, 0x0014CB12u);        /* ROM 0x14CB0A */
        return 0;
    }

    /* ROM 0x14CB18: Pal_QueueRamp(0x203C0, 0x900, 0x16, 0x20) and
     * Pal_QueueRamp(0x203C0, 0x3F80, 0x16, 0x20) */
    (void)Pal_QueueRamp(m, 0x000203C0u, 0x00000900u, 0x0016u, 0x0020u, 0x0016u);
    (void)Pal_QueueRamp(m, 0x000203C0u, 0x00003F80u, 0x0016u, 0x0020u, 0x0016u);
    /* ROM 0x14CB60: Pal_QueueRamp(0x14CE0A, 0, 0x16, 1) */
    (void)Pal_QueueRamp(m, 0x0014CE0Au, 0x00000000u, 0x0016u, 0x0001u, 0x0016u);

    /* ROM 0x14CB82 */
    wr16(m, obj + 0x06u, 0x0026u);
    wr32(m, obj + 0x00u, 0x0014CB96u);            /* ROM 0x14CB8E */
    return 0;
}
