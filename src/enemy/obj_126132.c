#include "obj_126132.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x126132: init scrolling/spawn offset and arm 0xB4-frame wait at 0x126158. */
int Obj_126132_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x30D2u, 0x008C0000u);
    wr32(m, F3_A5 + 0x30D6u, 0x012C0000u);
    Obj_11B39A(m, obj);
    wr16(m, obj + 0x06u, 0x00B4u);
    wr32(m, obj + 0x00u, 0x00126158u);
    return 0;
}
