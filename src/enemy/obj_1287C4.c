#include "obj_1287C4.h"
#include "../objects/obj_common.h"
#include "../player/player_move.h"

/* ROM 0x1287C4: spawn debris particle from parent + random angle/sprite */
int Obj_1287C4_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr16(m, obj + 0x24u, 1u);
    wr16(m, obj + 0x28u, 1u);
    wr32(m, obj + 0x14u, 0u);
    wr32(m, obj + 0x18u, 0u);
    uint16_t d0 = (uint16_t)((Rng_NextMem(m) & 0x000Fu) + 0x0020u);
    wr16(m, obj + 0x42u, d0);
    d0 = (uint16_t)(Rng_NextMem(m) & 0x003Fu);
    d0 = (d0 & 0xFF00u) | (uint8_t)(d0 - 0x60u);
    Obj_MoveByAngle(m, obj, d0);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0010u));
    uint16_t idx = (uint16_t)(Rng_NextMem(m) & 0x0003u);
    uint32_t tbl = 0x00128904u + (uint32_t)idx * 8u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, tbl + 0u));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, tbl + 1u));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, tbl + 2u));
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, tbl + 3u));
    wr32(m, obj + 0x10u, rd32(m, tbl + 4u));
    wr32(m, obj + 0x00u, 0x0012886Au);
    return 0;
}
