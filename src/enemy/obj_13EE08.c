#include "obj_13EE08.h"
#include "../objects/obj_common.h"

/* ROM 0x13EE08 */
int Obj_13EE08_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);

    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Cu));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Du));
    wr16(m, obj + 0x42u, (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x0Eu));

    for (uint32_t i = 0u; i < 10u; i++)
        wr32(m, obj + 0x44u + i * 4u, 0u);

    wr8(m, obj + 0x1Cu, 0x01u);
    wr16(m, obj + 0x06u, 0x0010u);
    wr32(m, obj + 0x00u, 0x0013EE50u);
    return 0;
}
