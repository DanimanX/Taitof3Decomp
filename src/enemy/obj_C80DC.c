#include "obj_C80DC.h"
#include "../player/player_move.h"

/* ROM 0x0C80DC: sparkle around the player at +0x0C (random speed, angle,
 * sprite, phase and radius) */
int Obj_C80DC_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x30u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x2Eu));
    wr16(m, obj + 0x42u, 0x00F0u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + (Rng_NextMem(m) & 0x1Fu)));
    uint16_t ang = (uint16_t)(uint8_t)((Rng_NextMem(m) & 0x1Fu) + 0x93u);
    wr16(m, obj + 0x20u, ang);
    Obj_MoveByAngle(m, obj, ang);
    wr32(m, obj + 0x10u, rd32(m, 0x000C81F8u + (Rng_NextMem(m) & 3u) * 4u));
    wr16(m, obj + 0x3Au, (uint16_t)(Rng_NextMem(m) & 0x80u));
    wr16(m, obj + 0x3Eu, (uint16_t)(Rng_NextMem(m) & 0x1Fu));
    wr16(m, obj + 0x22u, 0x1000u);
    wr32(m, obj + 0x00u, 0x000C8168u);
    return 0;
}
