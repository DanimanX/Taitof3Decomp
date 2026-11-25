#include "obj_124404.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x124404: init scrolling/spawn offset and arm 0x78-frame wait at 0x12442A. */
int Obj_124404_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x30D2u, 0x00000000u);
    wr32(m, F3_A5 + 0x30D6u, (uint32_t)(int32_t)-0x003C0000);
    Obj_11B39A(m, obj);
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x0012442Au);
    return 0;
}
