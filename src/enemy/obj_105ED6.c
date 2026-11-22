#include "obj_105ED6.h"

/* ROM 0x105ED6: enter state 0x105EDE (move.l #0x105ede,(A0)) */
int Obj_105ED6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x00105EDEu);
    return 0;
}
