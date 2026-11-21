#include "obj_12E21A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x12E21A */
int Obj_12E21A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12E21A: A1 = *(A5+0x32AA), then its +0x48 */
    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u);
    wr32(m, a1 + 0x5Au, rd32(m, a1 + 0x5Au) - 0x7DACu);
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) + 0x00B0u));
    if ((int16_t)rd16(m, F3_A5 + 0x32A2u) < 0)
        wr16(m, F3_A5 + 0x32A2u, 0x8000u);

    /* 0x12E23E */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12E212 */
        wr32(m, obj, 0x0012E21Au);
        return 0;
    }

    /* 0x12E244: free the parent first, then this object */
    Obj_Free(m, rd32(m, obj + 0x08u));
    wr8(m, F3_A5 + 0x30F2u, (uint8_t)(rd8(m, F3_A5 + 0x30F2u) | 0x04u)); /* bset.b #2 */
    Obj_Free(m, obj);
    return 0;
}
