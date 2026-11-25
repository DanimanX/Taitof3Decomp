#include "obj_1916BE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1916BE: attract/ending star/particle init */
int Obj_1916BE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, F3_A5 + 0x4B92u) + 1u);
    wr16(m, F3_A5 + 0x4B92u, cnt);
    if ((int16_t)cnt > 0x0A) {
        wr16(m, F3_A5 + 0x4B92u, (uint16_t)(cnt - 1u));
        Obj_Free(m, obj);
        return 0;
    }
    Obj_17A7FA(m, obj);
    wr16(m, obj + 0x50u, rd16(m, obj + 0x0Cu));
    wr16(m, obj + 0x54u, rd16(m, obj + 0x0Eu));
    wr32(m, obj + 0x10u, 0x0002DFB0u);
    wr8(m, obj + 0x1Fu, 0x60u);
    wr8(m, obj + 0x1Eu, 0xF0u);
    wr8(m, obj + 0x1Du, 0x7Cu);
    wr32(m, obj + 0x00u, 0x00191700u);
    return 0;
}
