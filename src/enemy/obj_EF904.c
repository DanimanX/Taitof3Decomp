#include "obj_EF904.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0EF904: seed from 0x0EFC02, set class colour, select script 0x0EFC3A
 * (or 0x0EFCAC if +0x10 != 0), set bit 2 of +0x34, hand over to 0x0EF938. */
int Obj_EF904_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_SeedFromParams(m, obj, 0x000EFC02u);
    Obj_ClassColour(m, obj);
    uint32_t script = (rd16(m, obj + 0x10u) == 0u) ? 0x000EFC3Au : 0x000EFCACu;
    Obj_StartScript(m, obj, script);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | (1u << 2)));
    wr32(m, obj + 0x00u, 0x000EF938u);
    return 0;
}
