#include "obj_13EE50.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24 */
static uint16_t sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0;

    for (;;) {
        d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* ROM 0x13EE50 */
int Obj_13EE50_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t prev_t = (int16_t)rd16(m, obj + 0x06u);
    wr16(m, obj + 0x06u, (uint16_t)(prev_t - 1));
    if (prev_t >= 1 && (rd8(m, obj + 0x07u) & 0x01u) != 0u) {
        for (uint32_t i = 0u; i < 10u; i++) {
            uint32_t a1 = obj + 0x44u + i * 4u;
            if (rd32(m, a1) == 0u) {
                uint32_t a2 = rd32(m, obj + 0x08u);
                uint16_t x = (uint16_t)(rd16(m, a2 + 0x14u) + rd16(m, obj + 0x2Cu) - 8u);
                uint16_t rng = (uint16_t)((rd16(m, F3_A5 - 0x1456u) & 0x03u) - 2u);
                uint16_t y = (uint16_t)(rng + rd16(m, a2 + 0x18u) + rd16(m, obj + 0x30u) - 8u);
                uint32_t d0 = ((uint32_t)(x & 0x3FFu) << 22) | ((uint32_t)(y & 0x1FFu) << 13);
                wr32(m, a1, d0);
                break;
            }
        }
    }

    for (uint32_t i = 0u; i < 10u; i++) {
        uint32_t a2 = obj + 0x44u + i * 4u;
        uint32_t slot = rd32(m, a2);
        if (slot == 0u)
            continue;

        uint16_t x_raw = (uint16_t)((slot >> 22) & 0x3FFu);
        int16_t x = (int16_t)((x_raw & 0x200u) ? (x_raw | 0xFC00u) : x_raw);
        wr16(m, obj + 0x14u, (uint16_t)x);
        x = (int16_t)((uint16_t)x - rd16(m, obj + 0x42u));
        slot = (slot & ~(0x3FFu << 22)) | (((uint32_t)(uint16_t)x & 0x3FFu) << 22);
        wr32(m, a2, slot);

        uint16_t y_raw = (uint16_t)((slot >> 13) & 0x1FFu);
        int16_t y = (int16_t)((y_raw & 0x100u) ? (y_raw | 0xFE00u) : y_raw);
        wr16(m, obj + 0x18u, (uint16_t)y);

        wr16(m, obj + 0x3Au, (uint16_t)((slot >> 5) & 0xFFu));
        wr16(m, obj + 0x3Eu, (uint16_t)(slot & 0x1Fu));

        uint16_t d0 = sub_0CBD24(m, obj, 0x0013EF3Cu);
        if ((int16_t)rd16(m, 0x0013EF3Cu + (uint32_t)(int32_t)(int16_t)d0) <= 0) {
            wr32(m, a2, 0u);
        } else {
            slot = (slot & ~(0xFFu << 5)) | (((uint32_t)rd16(m, obj + 0x3Au) & 0xFFu) << 5);
            slot = (slot & ~0x1Fu) | ((uint32_t)rd16(m, obj + 0x3Eu) & 0x1Fu);
            wr32(m, a2, slot);
            Obj_DrawSprite(m, obj);
        }
    }

    for (uint32_t i = 0u; i < 10u; i++) {
        if (rd32(m, obj + 0x44u + i * 4u) != 0u) {
            wr32(m, obj + 0x00u, 0x0013EE50u);
            return 0;
        }
    }

    Obj_Free(m, obj);
    return 0;
}
