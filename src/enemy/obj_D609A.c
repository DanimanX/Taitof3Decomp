#include "obj_D609A.h"
#include "../objects/obj_pool.h"

/* ROM 0x0D609A: move.l A0,-(SP) ; jsr 0x105EC ; addq #4,SP -- free self */
int Obj_D609A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
