#include "obj_E940A.h"
#include "obj_E8B72.h"

/* ROM 0x0E940A */
int Obj_E940A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0x000EABAAu);
    Obj_SetAnim(m, obj, 0x000E946Eu);
    wr32(m, obj + 0x00u, 0x000E9426u);
    return 0;
}
