#include "obj_116B86.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x116B86: tick for a 3D chain segment that tracks parent +0x50 bit 5
 * (freed when set), propagates damage/flash to parent via Obj_117058,
 * registers itself at A5+0x30B2 when within +-0x1000 of the front angle,
 * updates its 3D position via Obj_116BE6, and selects its sprite via Obj_117186. */
int Obj_116B86_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if ((rd8(m, a1 + 0x50u) & 0x20u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_117058(m, obj, a1);
    int16_t ang = (int16_t)((uint16_t)rd32(m, obj + 0x0Cu) + rd16(m, F3_A5 + 0x30BAu) + 0x4000u);
    if (ang >= -0x1000 && ang <= 0x1000)
        wr32(m, F3_A5 + 0x30B2u, obj);
    wr16(m, obj + 0x20u, rd16(m, F3_A5 + 0x30BCu));
    wr16(m, obj + 0x54u, rd16(m, F3_A5 + 0x30BEu));
    Obj_116BE6(m, obj);
    Obj_117186(m, obj, (uint16_t)rd32(m, obj + 0x0Cu), 0u);
    wr32(m, obj + 0x00u, 0x00116B86u);
    return 0;
}
