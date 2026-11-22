#include "obj_E7FC6.h"
#include "obj_E8B72.h"

/* ROM 0x0E7FC6 */
int Obj_E7FC6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMoverB(m, obj, 0x000E8120u);
    Obj_SetFrame(m, obj, 0x000E8150u);
    wr8(m, obj + 0x22u, rd8(m, rd32(m, obj + 0x08u) + 0x22u));
    wr8(m, obj + 0x23u, 0x1Cu);
    wr8(m, obj + 0x06u, 0x03u);
    Sound_PlayHooked(m, 0x002Au);
    wr32(m, obj + 0x00u, 0x000E800Cu);
    return 0;
}
