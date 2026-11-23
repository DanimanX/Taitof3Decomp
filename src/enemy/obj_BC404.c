#include "obj_BC404.h"
#include "../objects/obj_common.h"

/* ROM 0x0BC404: init from the table at 0xBC43C (lea (0x36,PC) at 0xBC404) */
int Obj_BC404_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x000BC43Cu;
    /* 0x0BC47C */
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x44u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x0086u);
    wr32(m, obj + 0x00u, 0x000BC4A8u);
    return 0;
}
