#include "obj_17F43C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x17F43C: polar placement + bounds check + spawn warning ring */
int Obj_17F43C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_PolarPlace(m, obj);
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < (int16_t)rd16(m, 0x001849F8u) || x > (int16_t)rd16(m, 0x001849FAu) ||
        y < (int16_t)rd16(m, 0x001849FCu) || y > (int16_t)rd16(m, 0x001849FEu)) {
        Obj_Free(m, obj);
        return 0;
    }
    (void)Obj_Spawn(m, obj, 0x0017CCBEu, 0x0Bu, 1u, 0x00140000u);
    wr16(m, obj + 0x06u, 0x0023u);
    wr32(m, obj + 0x00u, 0x0017F48Eu);
    return 0;
}
