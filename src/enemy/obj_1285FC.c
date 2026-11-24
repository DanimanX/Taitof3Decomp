#include "obj_1285FC.h"
#include "../objects/obj_common.h"

/* ROM 0x1285FC: the spawn argument packs four signed/unsigned 8-bit fields
 * from the MSB: dx:8 (@0), vy:8 (@8), speed:8 (@16), flip:1 (@24). */
int Obj_1285FC_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x0002B010u);
    Obj_ResetFields(m, obj);
    int32_t d0 = (int32_t)rd32(m, obj + 0x0Cu);
    int32_t d1 = d0 >> 24;                         /* bfexts {0:8} */
    wr16(m, obj + 0x50u, (uint16_t)d1);
    d1 = (d0 << 8) >> 24;                          /* bfexts {8:8} */
    wr16(m, obj + 0x52u, (uint16_t)d1);
    d1 = (d0 >> 8) & 0xFF;                         /* bfextu {16:8} */
    wr16(m, obj + 0x20u, (uint16_t)d1);
    d1 = (d0 >> 7) & 0x01;                         /* bfextu {24:1} */
    wr16(m, obj + 0x22u, (uint16_t)d1);
    wr16(m, obj + 0x2Cu, 0x0008u);
    wr16(m, obj + 0x30u, 0x0017u);
    wr16(m, obj + 0x2Eu, 0x0008u);
    wr16(m, obj + 0x32u, 0x0017u);
    wr16(m, obj + 0x28u, 0x0001u);
    wr16(m, obj + 0x42u, 0x0020u);
    wr16(m, obj + 0x06u, 0x00FFu);
    wr32(m, obj + 0x00u, 0x128660u);
    return 0;
}
