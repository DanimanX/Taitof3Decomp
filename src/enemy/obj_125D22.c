#include "obj_125D22.h"
#include "../objects/obj_common.h"

/* ROM 0x125D22 */
int Obj_125D22_Tick(const f3mem_t *m, uint32_t obj) {
    /* Reset object fields: clear +0x1C/+0x1E/+0x34/+0x3A/+0x3E (words) and +0x24/+0x28 (longs) */
    Obj_ResetFields(m, obj);

    /* Set image/animation index at obj+0x10 */
    wr32(m, obj + 0x10u, (uint32_t)0x4F1C8u);

    /* Clear additional fields: size flags and state */
    wr16(m, obj + 0x54u, 0x00u);
    wr16(m, obj + 0x56u, 0x00u);
    wr16(m, obj + 0x22u, 0x00u);
    wr32(m, obj + 0x00u, 0x00125D44u);

    return 0;
}
