#include "obj_11AD46.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x11AD46: 8-slot spark/explosion particle burst */
int Obj_11AD46_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x0002A108u);
    int16_t t = (int16_t)rd16(m, obj + 0x06u);
    if (t < 0x28 && (t & 1) == 0) {
        for (uint32_t i = 0u; i < 8u; i++) {
            uint32_t slot = obj + 0x44u + i * 4u;
            if (rd32(m, slot) == 0u) {
                uint32_t target = rd32(m, obj + 0x0Cu);
                uint32_t d1 = 1u;
                int16_t rx = (int16_t)(((uint16_t)Rng_NextMem(m) & 0x1Fu) - 0x14 + (int16_t)rd16(m, target + 0x14u));
                d1 = (d1 & 0x003FFFFFu) | ((uint32_t)(rx & 0x3FF) << 22);
                int16_t ry = (int16_t)(((uint16_t)Rng_NextMem(m) & 0x0Fu) + (int16_t)rd16(m, target + 0x18u));
                d1 = (d1 & 0xFFC01FFFu) | ((uint32_t)(ry & 0x1FF) << 13);
                wr32(m, slot, d1);
                break;
            }
        }
    }
    for (uint32_t i = 0u; i < 8u; i++) {
        uint32_t slot = obj + 0x44u + i * 4u;
        uint32_t v = rd32(m, slot);
        if (v == 0u)
            continue;
        int16_t px = (int16_t)((int32_t)v >> 22);
        wr16(m, obj + 0x14u, (uint16_t)px);
        px = (int16_t)(px - 5);
        v = (v & 0x003FFFFFu) | ((uint32_t)(px & 0x3FF) << 22);

        int16_t py = (int16_t)(((int32_t)(v << 10)) >> 23);
        wr16(m, obj + 0x18u, (uint16_t)py);
        py = (int16_t)(py + 1);
        v = (v & 0xFFC01FFFu) | ((uint32_t)(py & 0x1FF) << 13);

        uint8_t sc = (uint8_t)((v >> 5) & 0xFFu);
        wr8(m, obj + 0x1Fu, sc);
        wr8(m, obj + 0x1Eu, sc);
        sc = (uint8_t)(sc + 0x20u);
        v = (v & 0xFFFFE01Fu) | ((uint32_t)sc << 5);
        wr32(m, slot, v);

        Obj_DrawSprite(m, obj);
        uint16_t sum = (uint16_t)rd8(m, obj + 0x1Fu) + 0x20u;
        wr8(m, obj + 0x1Fu, (uint8_t)sum);
        if (sum > 0xFFu)
            wr32(m, slot, 0u);
    }
    int16_t next = (int16_t)(rd16(m, obj + 0x06u) + 1u);
    wr16(m, obj + 0x06u, (uint16_t)next);
    if (next <= 0x32) {
        wr32(m, obj + 0x00u, 0x0011AD46u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
