#include "obj_125DAA.h"

/* ROM 0x125DAA */
int Obj_125DAA_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x30EFu) & 0x01u) {
        wr32(m, obj, 0x00125DAAu);
        return 0;
    }
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj, 0x00125D44u);
    return 0;
}
