#include "obj_128D6C.h"
#include "../objects/obj_common.h"
#include "../player/player_move.h"

/* ROM 0x128D6C: init oriented projectile from parent + packed +0x0C */
int Obj_128D6C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
    uint32_t p = rd32(m, obj + 0x0Cu);
    int16_t dx = (int16_t)((int8_t)(p >> 24) - 8);
    int16_t dy = (int16_t)((int8_t)((p >> 16) & 0xFFu) - 8);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + dy));
    uint16_t ang = (uint16_t)(((p >> 8) & 0xFFu) << 8);
    uint16_t jitter = (uint16_t)((Rng_NextMem(m) & 0x1FFFu) - 0x1000u);
    ang = (uint16_t)(ang + jitter);
    wr16(m, obj + 0x20u, ang);
    wr16(m, obj + 0x24u, 1u);
    wr16(m, obj + 0x28u, 1u);
    wr16(m, obj + 0x2Cu, 0x000Bu);
    wr16(m, obj + 0x30u, 0x000Bu);
    wr16(m, obj + 0x2Eu, 0x0014u);
    wr16(m, obj + 0x32u, 0x0014u);
    wr8(m, obj + 0x4Fu, 0x4Cu);
    wr16(m, obj + 0x4Cu, 0x0014u);
    uint16_t d0 = (uint16_t)(ang + 0x0800u);
    uint16_t d1_q = (uint16_t)(d0 >> 14);
    wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, 0x000CC10Eu + d1_q) ^ 0x03u));
    uint16_t d1_oct = (uint16_t)((d0 >> 12) & 0x07u);
    wr32(m, obj + 0x0Cu, 0x00128FCEu + (uint32_t)(d1_oct * 3u) * 4u);
    wr8(m, obj + 0x1Du, 0x2Eu);
    wr16(m, obj + 0x42u, 0x0010u);
    Obj_MoveByAngle(m, obj, (uint16_t)(ang >> 8));
    wr16(m, obj + 0x06u, 3u);
    wr32(m, obj + 0x00u, 0x00128E44u);
    return 0;
}
