#include "obj_111A7A.h"
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

/* ROM 0x111F2C: d0 * ~(+0x1F) / 0xFF (word quotient) */
static uint16_t scale_111F2C(const f3mem_t *m, uint32_t obj, uint16_t d0)
{
    uint16_t d2 = (uint16_t)((uint8_t)~rd8(m, obj + 0x1Fu));
    return (uint16_t)(((uint32_t)d0 * d2) / 0xFFu);
}

/* ROM 0x111E26 (left, dir=-1) / 0x111EA8 (right, dir=+1): maybe spawn two fx objects */
static void spawn_fx(const f3mem_t *m, uint32_t obj, int right)
{
    uint32_t d0 = (uint32_t)(uint16_t)Rng_NextMem(m) * 2u;
    uint32_t d1 = (uint32_t)rd16(m, F3_A5 + 0x87Eu) + 0x8000u;
    if ((int32_t)d1 <= (int32_t)d0)
        return;
    uint16_t s = scale_111F2C(m, obj, 0x18);
    uint8_t b0 = right ? (uint8_t)(s + 0x30u) : (uint8_t)(0u - s);
    d1 = (d1 & 0x00FFFFFFu) | ((uint32_t)b0 << 24);
    d1 = (d1 & 0xFF00FFFFu) | (0x70u << 16);
    uint16_t r = (uint16_t)Rng_NextMem(m) & 0x7Fu;
    uint8_t b2 = right ? (uint8_t)(r - 0x40u) : (uint8_t)(r + 0x40u);
    d1 = (d1 & 0xFFFF00FFu) | ((uint32_t)b2 << 8);
    Obj_Spawn(m, obj, 0xD9056u, 9, 2, d1);
    Obj_Spawn(m, obj, 0xCDCA0u, 9, 2, d1);
}

/* ROM 0x111A7A: 0x1118BC variant that sheds fx and may arm 0x111B56 */
int Obj_111A7A_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t sx = (int16_t)rd16(m, SCROLL_X);
    uint16_t w = rd16(m, obj + 0x5Cu);
    if ((int16_t)(uint16_t)(w + 0x78u) <= sx) {
        Obj_Free(m, obj);
        return 0;
    }
    if ((int16_t)(uint16_t)(w - 0x1B8u) >= sx) {
        wr32(m, obj + 0x00u, 0x111A7Au);
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
    if ((int32_t)d0 > 0x180000) {
        sub_1119AC(m, obj, d0);
        spawn_fx(m, obj, 0);
    } else if ((int32_t)d0 < -0x180000) {
        sub_11196E(m, obj, d0);
        spawn_fx(m, obj, 1);
    }

    wr32(m, obj + 0x10u, rd32(m, obj + 0x60u));
    wr32(m, obj + 0x14u, y);
    wr8(m, obj + 0x1Fu, 0);
    wr8(m, obj + 0x1Cu, 0);
    if ((int16_t)rd16(m, obj + 0x24u) > 0) {
        Obj_0D0DC2(m, obj);
        flash_0CBE34(m, obj);
        Obj_DrawSprite(m, obj);
        wr32(m, obj + 0x00u, 0x111A7Au);
        return 0;
    }
    wr32(m, obj + 0x58u, Obj_0CBDDE(m, obj));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x68u, 0);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u));
    wr32(m, obj + 0x00u, 0x111B56u);
    return 0;
}
