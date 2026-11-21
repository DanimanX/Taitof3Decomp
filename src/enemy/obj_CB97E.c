#include "obj_CB97E.h"
#include "../objects/obj_common.h"

/* ROM 0x0CB97E */
int Obj_CB97E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    uint32_t a3 = 0x0061C000u;
    a3 += (uint32_t)(int32_t)(int16_t)rd16(m, a2 + (uint32_t)((int32_t)(int16_t)rd16(m, a1 + 0x22u) * 2));
    a3 += (uint32_t)((int32_t)(int16_t)rd16(m, a1 + 0x60u) * 4);
    wr32(m, obj + 0x0Cu, a3);
    /* jsr 0x55C8 (tilemap 0x10B74 -> A3) only writes playfield RAM */
    wr16(m, obj + 0x3Au, (uint16_t)(int16_t)-0x7E);
    wr16(m, obj + 0x3Eu, 0u);
    /* 0x0CB9C0 */
    Obj_Animate(m, obj, 0x000CBACCu);
    /* jsr 0x5A86 (char data -> 0x61E000) only writes character RAM */
    wr32(m, obj + 0x00u, 0x000CB9EEu);
    return 0;
}
