#include "obj_12442A.h"

/* ROM 0x12442A: idle down the +0x6 timer, stay on this entry while it is
 * still positive, else re-dispatch to the 0x124438 entry */
int Obj_12442A_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0012442Au);  /* 0x124422: stay */
        return 0;
    }
    wr32(m, obj + 0x00u, 0x00124438u);      /* 0x124430 */
    return 0;
}
