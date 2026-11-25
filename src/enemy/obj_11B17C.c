#include "obj_11B17C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x11B17C: boss/multi-part sub-object tick */
int Obj_11B17C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    if (rd8(m, parent + 0x59u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t tbl = rd32(m, obj + 0x0Cu);
    uint32_t cb = rd32(m, tbl);
    if (cb == 0x0011B06Eu) {
        Obj_11B06E(m, obj);
    } else if (cb == 0x0011B09Au) {
        Obj_11B09A(m, obj);
    } else if (cb == 0x0011B2CAu || cb == 0x0011B288u || cb == 0x0011B262u ||
               cb == 0x0011B2F6u || cb == 0x0011B31Cu) {
        /* None of these return -1; dispatch via Obj_Tick if needed */
        return -1;
    } else {
        return -1;
    }
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        Obj_11B06E(m, obj);
        Obj_11B09A(m, obj);
        wr32(m, obj + 0x00u, 0x0011B17Cu);
        return 0;
    }
    wr32(m, obj + 0x10u, rd32(m, tbl + 4u));
    uint32_t a1 = Obj_0CBDDE(m, obj);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr8(m, obj + 0x35u, 0u);
    wr32(m, obj + 0x00u, 0x0011B12Cu);
    return 0;
}
