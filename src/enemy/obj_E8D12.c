#include "obj_E8D12.h"
#include "obj_E8B72.h"

/* ROM 0x0E8D12: same body as 0x0E8B72 */
int Obj_E8D12_Tick(const f3mem_t *m, uint32_t obj)
{
    return Obj_E8B72_Tick(m, obj);
}
