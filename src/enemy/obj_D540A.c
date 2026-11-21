#include "obj_D540A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CBD86 */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x0CBE66 */
static void sub_0CBE66(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;

    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;

    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;

    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1));
    a1 += 1u;
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1));
    a1 += 1u;
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1));
    a1 += 1u;
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1));
    a1 += 1u;

    wr8(m, obj + 0x34u, rd8(m, a1));
    a1 += 1u;
    wr8(m, obj + 0x65u, rd8(m, a1));
    a1 += 1u;

    wr16(m, obj + 0x22u, rd16(m, obj + 0x0Eu));
    wr8(m, obj + 0x4Eu, rd8(m, obj + 0x0Cu));
}

/* ROM 0x0CBEB0 */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0)
{
    uint16_t d1 = rd16(m, F3_A5 + 0x087Eu);
    d1 = (uint16_t)(d1 << 3);
    d1 = (uint16_t)(d1 + 0x0C00u);
    int32_t prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)d1;
    uint32_t res = (uint32_t)prod << 4;
    res = (res >> 16) | (res << 16);
    return (uint16_t)res;
}

/* ROM 0x0CBEC2 */
static void sub_0CBEC2(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d0 * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)d0 * 2u);
    wr32(m, obj + 0x54u, (uint32_t)((int32_t)spd * (int32_t)c));
    wr32(m, obj + 0x58u, (uint32_t)((int32_t)spd * (int32_t)s));
}

/* ROM 0x0CBF7A */
static void sub_0CBF7A(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x00028740u);
    /* ROM 0x0CBF82 */
    wr32(m, obj, 0x000CBF8Au);
}

/* ROM 0x0CEE2C */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)rd8(m, obj + 0x4Eu);
    uint8_t col = rd8(m, 0x000CEE40u + (uint32_t)d0);
    wr8(m, obj + 0x1Du, col);
    wr8(m, obj + 0x4Fu, col);
}

/* ROM 0x0D540A */
int Obj_D540A_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    sub_0CBE66(m, obj, 0x000D53BCu);
    sub_0CEE2C(m, obj);

    wr16(m, obj + 0x60u, sub_0CBEB0(m, 0x0100u));
    wr16(m, obj + 0x62u, (uint16_t)(sub_0CBEB0(m, 0x001Eu) << 6));
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x5Eu, 0u);

    uint16_t d0 = rd16(m, obj + 0x22u);
    if (d0 != 0u) {
        uint16_t delay = (uint16_t)((int32_t)(int16_t)d0 * 10);
        wr16(m, obj + 0x06u, delay);
        if (delay != 0u) {
            /* ROM 0x0D545C */
            wr32(m, obj, 0x000D5464u);
            return 0;
        }
    }

    /* ROM 0x0D546A */
    uint32_t a1 = rd32(m, obj + 0x08u);
    a1 = rd32(m, a1 + 0x50u);
    if ((int16_t)rd16(m, a1 + 0x18u) < 0x6C) {
        /* ROM 0x0D548E */
        wr32(m, obj + 0x18u, 0x00D40000u);
        wr8(m, obj + 0x64u, 3u);
    } else {
        /* ROM 0x0D547C */
        wr32(m, obj + 0x18u, 0x00040000u);
        wr8(m, obj + 0x64u, 1u);
    }

    /* ROM 0x0D549C */
    uint16_t r = (uint16_t)Rng_NextMem(m) & 0x3Fu;
    r = (uint16_t)(r - 0x20u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + r));

    wr32(m, obj + 0x14u, 0xFFE00000u);
    wr16(m, obj + 0x20u, 0x0800u);

    sub_0CBEC2(m, obj);

    wr32(m, obj + 0x50u, 0x000D54D0u);
    sub_0CBF7A(m, obj);
    return 0;
}
