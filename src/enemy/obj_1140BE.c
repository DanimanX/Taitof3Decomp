#include "obj_1140BE.h"

/* ROM 0x1140BE */
int Obj_1140BE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x4Fu, 0u);
    wr16(m, obj + 0x3Au, 0u);

    /* 1140D2 moves (0xC,A0) into D0 (not D1), so 1140D6 masks the caller's D1w
     * dbra counter from Obj_RunGroups (31 - slot index). */
    uint16_t d1w = (uint16_t)((31u - rd8(m, obj + 0x05u)) & 0x0003u);
    wr16(m, obj + 0x3Eu, d1w);

    uint16_t d0w = (uint16_t)rd32(m, obj + 0x0Cu);
    d0w = (uint16_t)((uint16_t)(d0w * 5u) - 0x0024u);
    wr16(m, obj + 0x52u, d0w);
    wr16(m, obj + 0x54u, (uint16_t)-8);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr16(m, obj + 0x06u, 0x0014u);
    wr32(m, obj + 0x00u, 0x00114108u);
    return 0;
}
