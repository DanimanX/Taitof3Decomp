#include "obj_CB0DA.h"
#include "../objects/obj_pool.h"

/* ROM 0xCB0DA */
int Obj_CB0DA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0CB0DA: bclr.b #0,(A5+0x1D58) */
    wr8(m, F3_A5 + 0x1D58u, (uint8_t)(rd8(m, F3_A5 + 0x1D58u) & ~0x01u));
    /* 0x0CB0E0: bclr.b #0,(A1+0x64), A1 = (A0)+8 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr8(m, a1 + 0x64u, (uint8_t)(rd8(m, a1 + 0x64u) & ~0x01u));
    /* 0x0CB0EA: jsr Obj_Free */
    Obj_Free(m, obj);
    return 0;
}
