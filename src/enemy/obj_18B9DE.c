#include "obj_18B9DE.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

#define A5_(off) (F3_A5 + (off))

/* ROM 0x18BB3A: add (+0x64 * first word of the sprite def) >> 5 to +0x1A */
static void sub_18BB3A(const f3mem_t *m, uint32_t obj)
{
    uint16_t d1 = rd16(m, rd32(m, obj + 0x10u));
    uint32_t d0 = (uint32_t)rd8(m, obj + 0x64u) * d1;
    wr16(m, obj + 0x1Au, (uint16_t)(rd16(m, obj + 0x1Au) + (uint16_t)((uint16_t)d0 >> 5)));
}

/* ROM 0x17B02E: score receiver (1 -> (+0x36)->+8, 2 -> +0x36, else -1) */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u)
        return rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (k == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x18BB50: credit the receiver, bump the global score counters and
 * spawn the 0x179DEE burst + sound 0x13 */
static void sub_18BB50(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = sub_17B02E(m, obj);
    if ((int32_t)d0 >= 0)
        wr32(m, d0 + 0x48u, rd32(m, d0 + 0x48u) + 0x7530u);
    wr32(m, A5_(0x886u), rd32(m, A5_(0x886u)) + 0x12C5Fu);
    wr8(m, A5_(0x4B26u), (uint8_t)(rd8(m, A5_(0x4B26u)) | 0x20u));
    wr16(m, A5_(0x4B20u), rd16(m, obj + 0x34u));
    wr32(m, A5_(0x4B1Cu), rd32(m, obj + 0x36u));
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x04u));
    if (rd8(m, obj + 0x22u) != 0u)
        Obj_Spawn(m, obj, 0x179DEEu, 0, 1, 0);
    else
        Obj_Spawn(m, obj, 0x179DEEu, 5, 2, 0);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x13));
}

/* ROM 0x18BAAE: vtable tick of the spinning part */
static void tick_18BAAE(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= (int32_t)rd32(m, obj + 0x68u)) {
        wr32(m, obj + 0x10u, 0x4BCC8u);
        wr16(m, obj + 0x16u, 0x28u);
        wr16(m, obj + 0x1Au, 0xFFF0u);
        wr32(m, obj + 0x2Cu, 0x80030u);
        wr32(m, obj + 0x30u, 0x100048u);
        wr32(m, obj + 0x68u, 0u);
        sub_18BB3A(m, obj);
        sub_18BB50(m, obj);
    }
    uint16_t d0 = rd16(m, 0x27E40u + (uint32_t)rd8(m, obj + 0x5Eu) * 2u);
    d0 = (uint16_t)((uint16_t)(d0 + 0x4000u) >> 8);
    wr8(m, obj + 0x1Fu, (uint8_t)(d0 + rd8(m, obj + 0x64u)));
    wr16(m, obj + 0x5Eu, (uint16_t)(rd16(m, obj + 0x5Eu) + 0x200u));
}

/* ROM 0x18BB12: vtable death handler */
static void death_18BB12(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x4BD2Cu);
    wr16(m, obj + 0x16u, 0x20u);
    wr16(m, obj + 0x1Au, 0xFFF8u);
    sub_18BB3A(m, obj);
    sub_18BB50(m, obj);
    wr8(m, A5_(0x4B7Cu), (uint8_t)(rd8(m, A5_(0x4B7Cu)) | 0x80u));
    wr32(m, obj, 0x17AA36u);
}

/* ROM 0x17BA76: pick the point the 0x17A122 burst is aimed from; -1 = none.
 * ox and oy get D0.w / D1.w. */
static int sub_17BA76(const f3mem_t *m, uint32_t obj, uint16_t *ox, uint16_t *oy)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 2u) {
        if (rd16(m, A5_(0x878u)) % 3u != 0u)
            return -1;
        uint16_t d2 = (uint16_t)((uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)));
        uint16_t d3 = (uint16_t)((uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u)));
        d2 = (uint16_t)((int16_t)d2 >> 1);
        d3 = (uint16_t)((int16_t)d3 >> 1);
        uint16_t r = Obj_RandRange(m, d3);
        *oy = (uint16_t)(r + rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u) + d3);
        r = Obj_RandRange(m, d2);
        *ox = (uint16_t)(r + rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu) + d2);
        return 0;
    }
    uint8_t k = rd8(m, a1 + 0x22u);
    if (k != 0u && k != 4u && (rd16(m, A5_(0x878u)) & 3u) != 0u)
        return -1;
    int16_t x0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu));
    int16_t y0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u));
    int16_t x1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Cu));
    int16_t y1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x30u));
    int16_t d4 = x1 > x0 ? x1 : x0;
    int16_t d5 = y1 > y0 ? y1 : y0;
    x0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Eu));
    y0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x32u));
    x1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Eu));
    y1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x32u));
    int16_t d6 = x1 < x0 ? x1 : x0;
    int16_t d7 = y1 < y0 ? y1 : y0;
    *ox = (uint16_t)(d4 + (uint16_t)((uint16_t)(d6 - d4) >> 1));
    *oy = (uint16_t)(d5 + (uint16_t)((uint16_t)(d7 - d5) >> 1));
    return 0;
}

/* ROM 0x17B9BA: occasionally fire a 0x17A122 burst at the target */
static void sub_17B9BA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    if (Obj_RandRange(m, 5) != 0u)
        return;
    uint16_t x, y;
    if (sub_17BA76(m, obj, &x, &y) < 0)
        return;
    x = (uint16_t)(x - rd16(m, obj + 0x50u));
    y = (uint16_t)(y - rd16(m, obj + 0x54u));
    Obj_Spawn(m, obj, 0x17A122u, 5, 2, ((uint32_t)x << 16) | y);
}

/* ROM 0x18B9DE: part tick -- vtable tick/death, follow the parent, draw */
int Obj_18B9DE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        if (rd32(m, a3 + 0x0Cu) != 0x18BB12u)
            return -1;
        death_18BB12(m, obj);
        return 0;
    }
    if (rd32(m, a3 + 0x08u) != 0x18BAAEu)
        return -1;
    uint16_t idx = rd16(m, obj + 0x40u);
    tick_18BAAE(m, obj);
    if ((rd8(m, A5_(0x4B3Bu)) & 0x01u) == 0u) {
        Obj_0D0DC2(m, obj);
        if ((rd8(m, obj + 0x34u) & 0x02u) != 0u) {
            sub_17B9BA(m, obj);
            wr8(m, obj + 0x35u, 0u);
        }
    }
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);
    if (Obj_AnimTick(m, obj, idx) < 0)
        return -1;
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t w = rd16(m, a1);
    uint16_t h = rd16(m, a1 + 2u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(h << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(w << 3)));
    if (rd8(m, obj + 0x1Du) == 6u)
        wr8(m, obj + 0x1Du, 8u);
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, 6u);
    else
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x66u));
    wr8(m, obj + 0x35u, 0u);
    Obj_DrawSprite(m, obj);
    wr32(m, obj, 0x18B9DEu);
    return 0;
}
