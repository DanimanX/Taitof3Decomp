#include "obj_111B56.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

#define SCROLL_X (F3_A5 - 0x5DA8u)

/* 68000 DIVS.W #d: on overflow D0 is left unchanged, else rem:quot */
static uint32_t divs_w(uint32_t d0, int16_t d)
{
    int32_t n = (int32_t)d0;
    int32_t q = n / d;
    int32_t r = n % d;
    if (q < -32768 || q > 32767)
        return d0;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* ROM 0x11196E: lean left by how far d0 lies below -0x180000 */
static void sub_11196E(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    d0 = 0u - (d0 + 0x180000u);
    if ((int32_t)d0 >= 0xE80000) {
        wr8(m, obj + 0x1Fu, 0);
    } else {
        uint16_t q = (uint16_t)(d0 / 0xE8u);
        wr8(m, obj + 0x1Fu, (uint8_t)((uint16_t)(0u - q) >> 8));
    }
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x30u));
    wr8(m, obj + 0x1Cu, 0);
    wr32(m, obj + 0x10u, rd32(m, obj + 0x64u));
    Obj_DrawSprite(m, obj);
}

/* ROM 0x1119AC: lean right by how far d0 lies above +0x180000 */
static void sub_1119AC(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    d0 -= 0x180000u;
    if ((int32_t)d0 >= 0xE80000) {
        wr8(m, obj + 0x1Fu, 0);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x48u));
    } else {
        uint16_t q = (uint16_t)(d0 / 0xE8u);
        wr8(m, obj + 0x1Fu, (uint8_t)((uint16_t)(0u - q) >> 8));
        uint16_t hi = (uint16_t)(((uint32_t)q * 0x48u) >> 16);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - hi));
    }
    wr8(m, obj + 0x1Cu, 1);
    wr32(m, obj + 0x10u, rd32(m, obj + 0x64u));
    Obj_DrawSprite(m, obj);
}

/* random spark position packed as {x, y, 0, 2} for the 0x13E4EE spawn */
static void spawn_spark(const f3mem_t *m, uint32_t obj)
{
    uint32_t d1 = 2u;
    uint16_t x = (uint16_t)(((uint32_t)(uint16_t)Rng_NextMem(m) * 0x2Fu) >> 16);
    d1 = (d1 & 0x00FFFFFFu) | ((uint32_t)(uint8_t)(x - 8u) << 24);
    uint16_t y = (uint16_t)(((uint32_t)(uint16_t)Rng_NextMem(m) * 0x74u) >> 16);
    d1 = (d1 & 0xFF00FFFFu) | ((uint32_t)(uint8_t)(y - 8u) << 16);
    Obj_Spawn(m, obj, 0x13E4EEu, 0, 1, d1);
}

/* ROM 0x111B56: collapsing scenery piece -- sparks, then a final burst */
int Obj_111B56_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x18u, 0);
    int16_t sx = (int16_t)rd16(m, SCROLL_X);
    if ((int16_t)(uint16_t)(rd16(m, obj + 0x5Cu) + 0x78u) <= sx) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t d0 = rd32(m, obj + 0x5Cu) - rd32(m, SCROLL_X) - 0x9F8000u;
    d0 = divs_w(d0, 0x280);
    d0 = (uint32_t)((int32_t)(int16_t)d0 * 0x7A4);
    uint32_t y = d0 + 0x880000u;
    y += (uint32_t)((int32_t)(int16_t)Rng_NextMem(m) * 4);
    wr32(m, obj + 0x14u, y);

    d0 = rd32(m, obj + 0x5Cu) - rd32(m, SCROLL_X) - 0x9F8000u;
    d0 = divs_w(d0, 0x280);
    d0 = (uint32_t)((int32_t)(int16_t)d0 * 0x7A4);
    if ((int32_t)d0 > 0x180000)
        sub_1119AC(m, obj, d0);
    else if ((int32_t)d0 < -0x180000)
        sub_11196E(m, obj, d0);

    wr32(m, obj + 0x10u, rd32(m, obj + 0x60u));
    wr32(m, obj + 0x14u, y);
    wr8(m, obj + 0x1Fu, 0);
    wr8(m, obj + 0x1Cu, 0);
    Obj_DrawSprite(m, obj);

    if ((rd16(m, obj + 0x68u) & 1u) == 0u) {
        spawn_spark(m, obj);
        Sound_PlayHooked(m, 0x35);
    } else {
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + 0x740000u);
        spawn_spark(m, obj);
    }

    uint16_t n = (uint16_t)(rd16(m, obj + 0x68u) + 1u);
    wr16(m, obj + 0x68u, n);
    if ((int16_t)n < 0x5A || (int16_t)rd16(m, obj + 0x14u) >= 0x87) {
        wr32(m, obj + 0x00u, 0x111B56u);
        return 0;
    }
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x6Au) << 6));
    Obj_Spawn(m, obj, 0x13E4EEu, 5, 2, 0x00C00005u);
    uint16_t k = (uint16_t)(rd16(m, obj + 0x6Au) + 1u);
    wr16(m, obj + 0x6Au, k);
    if ((int16_t)k <= 4) {
        wr32(m, obj + 0x00u, 0x111B56u);
        return 0;
    }
    Sound_PlayHooked(m, 0x16);
    wr32(m, obj + 0x00u, 0x111D18u);
    return 0;
}
