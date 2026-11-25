#include "obj_115304.h"
#include "../objects/obj_pool.h"

/* ROM 0x115304: same shape as 0x113A2C (count down, flag the parent, free it). */
int Obj_115304_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x115304: subq.w #0x1,(0x6,A0) ; bgt.b 0x1152fc */
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x00115304u);          /* 0x1152fc */
        return 0;
    }

    /* 0x11530A: bset.b #0x5,(0x50,A0) */
    uint8_t b = rd8(m, obj + 0x50u);
    wr8(m, obj + 0x50u, (uint8_t)(b | 0x20u));

    /* 0x115310: move.l (0x8,A0),-(SP) ; jsr 0x105EC ; lea (0x4,SP),SP */
    uint32_t a4 = rd32(m, obj + 0x08u);
    (void)Obj_Free(m, a4);

    wr32(m, obj + 0x00u, 0x00115326u);
    return 0;
}
