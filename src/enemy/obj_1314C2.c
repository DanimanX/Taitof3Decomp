#include "obj_1314C2.h"
#include "../objects/obj_common.h"

/* ROM 0x1314C2 */
int Obj_1314C2_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr8(m, obj + 0x4Fu, 0x00u);

    uint32_t d0 = Rng_NextMem(m) & 0xFFFFu;
    d0 = (uint32_t)((uint16_t)(d0 / 0x2Du)) | ((d0 % 0x2Du) << 16);
    wr16(m, obj + 0x16u, (uint16_t)(((uint16_t)(d0 >> 16) & 0x00FCu) + 0x0018u));

    d0 = Rng_NextMem(m) & 0xFFFFu;
    d0 = (uint32_t)((uint16_t)(d0 / 0x30u)) | ((d0 % 0x30u) << 16);
    wr16(m, obj + 0x1Au, (uint16_t)(((uint16_t)(d0 >> 16) & 0x00FCu) + 0x0028u));

    wr32(m, obj + 0x10u, 0x000316D4u);
    wr16(m, obj + 0x1Eu, (uint16_t)(int16_t)-0x0F10);
    wr32(m, obj + 0x00u, 0x0013151Eu);
    return 0;
}
