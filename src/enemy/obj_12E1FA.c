#include "obj_12E1FA.h"
#include "../objects/obj_common.h"

/* ROM 0x12E1FA: timed hold (obj+0x06 frames) then arm the 0x12E21A state and
 * point the global sprite banks at 0x10 / 2. */
int Obj_12E1FA_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        wr32(m, obj + 0x00u, 0x0012E1FAu);
        return 0;
    }
    wr16(m, F3_A5 + 0x32A6u, 0x0010u);
    wr16(m, F3_A5 + 0x32A8u, 0x0002u);
    wr16(m, obj + 0x06u, 0x006Eu);
    wr32(m, obj + 0x00u, 0x0012E21Au);
    return 0;
}
