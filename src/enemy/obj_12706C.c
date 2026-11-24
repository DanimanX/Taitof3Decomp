#include "obj_12706C.h"
#include "../objects/obj_common.h"

/* ROM 0x12706C */
int Obj_12706C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr16(m, obj + 0x2Cu, 0x0000u);
    wr16(m, obj + 0x30u, 0x0026u);
    wr16(m, obj + 0x2Eu, 0x004Bu);
    wr16(m, obj + 0x32u, 0x0036u);
    wr16(m, obj + 0x24u, 0x0000u);
    wr16(m, obj + 0x28u, 0x0001u);
    wr16(m, obj + 0x50u, 0x0038u);
    wr16(m, obj + 0x52u, (uint16_t)-0x28);
    wr16(m, obj + 0x54u, 0x0000u);
    wr16(m, obj + 0x56u, 0x0000u);
    wr8(m, obj + 0x1Du, 0x00u);                                  /* sf */
    wr8(m, obj + 0x4Fu, 0x00u);                                  /* sf */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* bset #4 */
    wr32(m, obj + 0x00u, 0x1270BEu);
    return 0;
}
