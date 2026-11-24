#include "obj_1281C4.h"
#include "../objects/obj_common.h"

/* ROM 0x1281C4: the spawn argument packs the sprite def in the low word and
 * the sprite index in bits 16..20 (swap + asl.w #4). */
int Obj_1281C4_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x52u, (uint16_t)(d0 & 0xFFFFu));
    d0 = (d0 << 16) | (d0 >> 16);                  /* swap D0 */
    uint16_t idx = (uint16_t)((uint16_t)(d0 & 0xFFFFu) << 4); /* asl.w #4 */
    wr16(m, obj + 0x22u, idx);
    wr16(m, obj + 0x54u, 0x0000u);
    wr16(m, obj + 0x56u, 0x0000u);
    wr8(m, obj + 0x1Du, 0x00u);                    /* sf */
    wr8(m, obj + 0x4Fu, 0x00u);                    /* sf */
    wr16(m, obj + 0x24u, 0x0014u);
    wr16(m, obj + 0x28u, 0x0001u);
    wr16(m, obj + 0x2Cu, 0x000Cu);
    wr16(m, obj + 0x30u, 0x0004u);
    wr16(m, obj + 0x2Eu, 0x0021u);
    wr16(m, obj + 0x32u, 0x000Bu);
    wr16(m, obj + 0x4Cu, 0x7530u);
    wr16(m, obj + 0x06u, 0x007Au);
    wr32(m, obj + 0x36u, 0u);
    wr32(m, obj + 0x00u, 0x128226u);
    return 0;
}
