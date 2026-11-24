#include "obj_128476.h"
#include "../objects/obj_common.h"

/* ROM 0x128476 */
int Obj_128476_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);

    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)((int16_t)((d0 >> 24) & 0xFFu) - 8));
    wr16(m, obj + 0x52u, (uint16_t)((int16_t)((d0 >> 16) & 0xFFu) - 8));
    wr16(m, obj + 0x42u, (uint16_t)(((d0 >> 8) & 0xFFu) << 2));
    wr16(m, obj + 0x22u, (uint16_t)((d0 & 0xFFu) << 3));

    wr16(m, obj + 0x2Cu, 0x0003u);
    wr16(m, obj + 0x30u, 0x0003u);
    wr16(m, obj + 0x2Eu, 0x000Cu);
    wr16(m, obj + 0x32u, 0x000Cu);
    wr16(m, obj + 0x24u, 0x0001u);
    wr16(m, obj + 0x28u, 0x0001u);

    wr32(m, obj + 0x00u, 0x001284D4u);
    return 0;
}
