#include "obj_1118BC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
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

/* ROM 0x0CBE34: flash byte +0x1D (0x40/0x42) while +0x35 is set, else restore from +0x4F */
static void flash_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    } else if (rd8(m, obj + 0x1Du) == 0x40u) {
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, 0x42u);
    } else {
        wr8(m, obj + 0x1Du, 0x40u);
    }
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

/* ROM 0x1118BC: perspective-scaled scenery piece tied to the scroll position */
int Obj_1118BC_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t sx = (int16_t)rd16(m, SCROLL_X);
    uint16_t w = rd16(m, obj + 0x5Cu);
    if ((int16_t)(uint16_t)(w + 0x78u) <= sx) {
        Obj_Free(m, obj);
        return 0;
    }
    if ((int16_t)(uint16_t)(w - 0x1B8u) >= sx) {
        wr32(m, obj + 0x00u, 0x1118BCu);
        return 0;
    }
    uint32_t d0 = rd32(m, obj + 0x5Cu) - rd32(m, SCROLL_X) - 0x9F8000u;
    d0 = divs_w(d0, 0x280);
    d0 = (uint32_t)((int32_t)(int16_t)d0 * 0x7A4);
    uint32_t y = d0 + 0x880000u;
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
    Obj_0D0DC2(m, obj);
    flash_0CBE34(m, obj);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x1118BCu);
    return 0;
}
