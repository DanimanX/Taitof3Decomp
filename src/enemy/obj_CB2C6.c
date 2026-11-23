#include "obj_CB2C6.h"
#include "../objects/obj_common.h"

/* ROM 0x0CB2C6: scrolling text line (colour 0x2E, drift 0x7800, 0xC0 frames). */
int Obj_CB2C6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr8(m, obj + 0x1Du, 0x2Eu);
    wr16(m, obj + 0x20u, rd16(m, obj + 0x0Eu));
    wr16(m, obj + 0x22u, 0x7800u);
    wr16(m, obj + 0x06u, 0x00C0u);
    wr32(m, obj + 0x00u, 0x000CB2ECu);
    return 0;
}
