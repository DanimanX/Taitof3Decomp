#include "obj_BC03C.h"
#include "../objects/obj_common.h"

/* ROM 0x0BC03C: init */
int Obj_BC03C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x10u, 0x000548E8u);
    wr16(m, obj + 0x14u, 0x0010u);
    wr16(m, obj + 0x18u, 0x0060u);
    wr16(m, obj + 0x60u, 0x007Fu);
    wr32(m, obj + 0x00u, 0x000BC064u);
    return 0;
}
