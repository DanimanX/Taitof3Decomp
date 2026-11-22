#include "obj_125CA8.h"
#include "../objects/obj_pool.h"

/* ROM 0x125CA8: branch thunk to 0x125CFA (retire the object) */
int Obj_125CA8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x125CA8 bra.w 0x125CFA: move.l A0,-(SP); jsr 0x105EC; rts */
    Obj_Free(m, obj);
    return 0;
}
