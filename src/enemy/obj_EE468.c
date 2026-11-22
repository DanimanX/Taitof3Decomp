#include "obj_EE468.h"
#include "obj_EE0A4.h"

/* ROM 0x0EE468: same code as 0x0EDC38 */
int Obj_EE468_Tick(const f3mem_t *m, uint32_t obj)
{
    return Enemy_FormationShooter(m, obj, 0x000EDEC2u, 0x000EE758u);
}
