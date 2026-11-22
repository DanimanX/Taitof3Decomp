#include "obj_E953A.h"
#include "obj_E8B72.h"

/* ROM 0x0E953A */
int Obj_E953A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMoverB(m, obj, 0x000EABAAu);
    Obj_SetAnim(m, obj, 0x000E9590u);
    wr32(m, obj + 0x00u, 0x000E9556u);
    return 0;
}
