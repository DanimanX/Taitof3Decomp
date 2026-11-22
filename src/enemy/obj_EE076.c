#include "obj_EE076.h"
#include "obj_E8B72.h"

/* ROM 0x0EE076 */
int Obj_EE076_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0x000EDEC2u);
    Obj_InitPalette(m, obj);
    Obj_SetScript(m, obj, 0x000EE32Eu);
    Obj_ResetAnimKey(m, obj);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000EE0A4u);
    return 0;
}
