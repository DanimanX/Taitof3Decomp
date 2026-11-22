#include "obj_124524.h"

/* ROM 0x124524: idle down the +0x6 timer, stay on this entry while it is
 * still positive, else re-dispatch to the 0x124532 entry */
int Obj_124524_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x00124524u);  /* 0x12451C: stay */
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00124532u);      /* 0x12452A */
    return 0;
}
