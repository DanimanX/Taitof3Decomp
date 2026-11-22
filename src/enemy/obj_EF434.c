#include "obj_EF434.h"
#include "obj_E8B72.h"

/* ROM 0x0EF434 */
int Obj_EF434_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u)
        Obj_SetAnim(m, obj, 0x000EF7BEu);
    wr32(m, obj + 0x00u, 0x000EF44Eu);
    return 0;
}
