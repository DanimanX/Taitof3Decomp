#include "obj_11BD96.h"
#include "../objects/obj_pool.h"

/* ROM 0x0011BD8E: re-arm the tick vector with 0x11BD96. */
static void Obj_11BD96_SetVector(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x0011BD96u);
}

/* ROM 0x11BD96: wait +0x6 frames, then free the object. */
int Obj_11BD96_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11BD96: subq.w #1,(0x6,A0); bgt.b 0x11BD8E */
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_11BD96_SetVector(m, obj);
        return 0;
    }

    /* 0x11BD9C: move.l A0,-(SP); jsr 0x0105EC; rts */
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
