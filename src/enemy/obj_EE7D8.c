#include "obj_EE7D8.h"
#include "obj_EE0A4.h"

/* ROM 0x0EE7D8: same code as 0x0EDC38 */
int Obj_EE7D8_Tick(const f3mem_t *m, uint32_t obj)
{
    return Enemy_FormationShooter(m, obj, 0x000EDEC2u, 0x000EEAC8u);
}
