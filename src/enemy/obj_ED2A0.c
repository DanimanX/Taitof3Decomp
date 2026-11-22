#include "obj_ED2A0.h"
#include "obj_E8B72.h"

/* ROM 0x0ED2A0 */
int Obj_ED2A0_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0x000ED56Au);
    Obj_InitPalette(m, obj);
    Obj_SetScript(m, obj, 0x000ED5A2u);
    Obj_SetAnim(m, obj, 0x000ED5FCu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000ED2D8u);
    return 0;
}
