#include "obj_1302B8.h"
#include "../objects/obj_pool.h"

/* ROM 0x1302B8 */
int Obj_1302B8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x00409D2Cu;
    int16_t d7 = (int16_t)rd16(m, F3_A5 + 0x1D34u);        /* 0x1302BE */
    for (;;) {
        int lt = d7 < 1;                                   /* 0x1302C2 subq.w/blt */
        d7 = (int16_t)(d7 - 1);
        if (lt) break;

        uint32_t a2 = rd32(m, a1);                         /* 0x1302C8 movea.l (A1)+ */
        a1 += 4u;

        uint32_t d0 = Rng_NextMem(m) & 0xFFFFu;            /* 0x1302CA jsr 0x1D8A */
        d0 = (d0 << 3) + 0x8000u;                          /* 0x1302D6/0x1302D8 */
        wr32(m, a2 + 0x14u, rd32(m, a2 + 0x14u) - d0);     /* 0x1302DE */
    }
    /* 0x1302E4 */
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x1302E8 bgt */
        /* 0x1302B0 */
        wr32(m, obj + 0x00u, 0x001302B8u);
        return 0;
    }
    Obj_Free(m, obj);                                      /* 0x1302EA */
    return 0;
}
