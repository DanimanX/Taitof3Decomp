#include "obj_EF400.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"

/* ROM 0x0EF400 */
int Obj_EF400_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0x000EF672u);
    Obj_InitPalette(m, obj);
    Obj_SetAnim(m, obj, 0x000EF6AAu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x01u));
    Coll_AddHazard(m, obj);
    wr32(m, obj + 0x00u, 0x000EF434u);
    return 0;
}
