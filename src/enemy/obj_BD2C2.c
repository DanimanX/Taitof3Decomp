#include "obj_BD2C2.h"

/* ROM 0x0BD2C2: scroll the three layers, then switch to 0xBD308 */
int Obj_BD2C2_Tick(const f3mem_t *m, uint32_t obj)
{
    static const uint16_t off[6] = { 0x4230u, 0x4238u, 0x4240u, 0x422Cu, 0x4234u, 0x423Cu };
    for (int i = 0; i < 6; i++) {
        uint32_t k = i < 3 ? 0x02A0u : 0x07D0u;
        wr32(m, F3_A5 + off[i], rd32(m, F3_A5 + off[i]) - k);
    }
    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BD2C2u);
        return 0;
    }
    /* move.w #0x30,(0x6,A0); beq: never taken (0x30 != 0), so the
     * playfield blit at 0x0BD30E is dead code */
    wr16(m, obj + 0x06u, 0x0030u);
    wr32(m, obj + 0x00u, 0x000BD308u);
    return 0;
}
