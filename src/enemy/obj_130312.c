#include "obj_130312.h"
#include "../kernel/rng.h"
#include "../objects/obj_pool.h"

/* ROM 0x130312 */
int Obj_130312_TickD3(const f3mem_t *m, uint32_t obj, uint32_t d3)
{
    uint16_t d0 = (uint16_t)((Rng_NextMem(m) & 0x7Fu) + 0x3Cu);
    /* bfins D0,D3{0:10} ; bfins D1(=0xBB),D3{10:10} ; bfins D2(=0, sf),D3{20:4}:
     * bits 31..8 are rebuilt, bits 7..0 keep the caller's D3 */
    uint32_t arg = ((uint32_t)(d0 & 0x3FFu) << 22) | ((uint32_t)0x00BBu << 12) | (d3 & 0xFFu);
    Obj_Spawn(m, obj, 0x00118E88u, 5u, 2u, arg);
    int16_t c = (int16_t)(rd16(m, obj + 0x06u) - 1);
    wr16(m, obj + 0x06u, (uint16_t)c);
    if (c > 0) {
        wr32(m, obj, 0x00130312u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}

/* Without the caller's registers: 0x42 is the value most captures show. */
int Obj_130312_Tick(const f3mem_t *m, uint32_t obj)
{
    return Obj_130312_TickD3(m, obj, 0x42u);
}
