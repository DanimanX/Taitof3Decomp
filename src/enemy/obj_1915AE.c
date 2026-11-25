#include "obj_1915AE.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1915AE: particle spawner init */
int Obj_1915AE_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_17A7FA(m, obj);
    wr16(m, obj + 0x14u, 0x013Au);
    wr16(m, obj + 0x18u, 0x00A8u);
    wr16(m, F3_A5 + 0x4B92u, 0u);
    wr16(m, obj + 0x22u, 0x0010u);
    uint16_t r = (uint16_t)(Obj_RandRange(m, 5u) + 5u);
    wr16(m, F3_A5 + 0x0006u, r);
    wr32(m, obj + 0x00u, 0x001915E0u);
    return 0;
}
