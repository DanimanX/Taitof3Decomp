#include "obj_119CBA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x119CBA */
int Obj_119CBA_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x330Cu) < 0x12C0) {
        wr32(m, obj + 0x00u, 0x00119CBAu);
        return 0;
    }

    uint32_t a0 = 0x00627600u + (uint32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t d7 = 0; d7 <= 0xE7u; d7++) {
        wr16(m, a0, 0x7CCCu);
        a0 += 2u;
    }

    wr32(m, obj + 0x14u, 0xFFD60000u);
    wr32(m, obj + 0x18u, 0x00610000u);
    wr32(m, obj + 0x0Cu, 0x00001000u);
    wr16(m, obj + 0x06u, 0x0078u);
    wr32(m, obj + 0x00u, 0x00119D06u);
    return 0;
}
