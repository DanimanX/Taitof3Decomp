#include "obj_ED77A.h"
#include "obj_E8B72.h"

/* ROM 0x0ED77A */
int Obj_ED77A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0x000EDA1Eu);
    Obj_InitPalette(m, obj);
    Obj_SetScript(m, obj, 0x000EDA56u);
    Obj_SetAnim(m, obj, 0x000EDAB0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000ED7ACu);
    return 0;
}
