#include "obj_D5974.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBD86: clear animation / motion state */
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

/* ROM 0x0CBE66: hit/collision record at A1 */
static void sub_0CBE66(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr8(m, obj + 0x34u, rd8(m, a1)); a1++;
    wr8(m, obj + 0x65u, rd8(m, a1));
    wr16(m, obj + 0x22u, rd16(m, obj + 0x0Eu));
    wr8(m, obj + 0x4Eu, rd8(m, obj + 0x0Cu));
}

/* ROM 0x0CBEB0: D0w scaled by the difficulty (A5+0x87E) */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0)
{
    uint16_t d1 = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    uint32_t res = (uint32_t)((int32_t)(int16_t)d0 * (int32_t)(int16_t)d1) << 4;
    return (uint16_t)(res >> 16);
}

/* ROM 0x0CEE2C */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = rd8(m, obj + 0x4Eu);
    uint8_t b = rd8(m, 0x000CEE40u + (uint32_t)(int32_t)(int16_t)d0);
    wr8(m, obj + 0x1Du, b);
    wr8(m, obj + 0x4Fu, b);
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
    wr32(m, obj + 0x10u, 0x28740u);
    wr32(m, obj, 0xCBF8Au);
}


/* ROM 0x0D5974 */
int Obj_D5974_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    sub_0CBE66(m, obj, 0xD5932u);
    sub_0CEE2C(m, obj);
    uint32_t p = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    wr16(m, obj + 0x60u, sub_0CBEB0(m, 0x100u));
    wr16(m, obj + 0x42u, sub_0CBEB0(m, 0x10u));
    wr16(m, obj + 0x20u, 0x8000u);
    sub_0CBEC2(m, obj);
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x5Eu, 0u);
    wr32(m, obj + 0x3Au, 0x10000u);
    wr8(m, obj + 0x64u, 0u);
    uint16_t d = rd16(m, obj + 0x22u);
    if (d != 0u) {
        d = (uint16_t)((int32_t)(int16_t)d * 0x14);
        wr16(m, obj + 0x06u, d);
        if (d != 0u) {
            wr32(m, obj, 0xD59F0u);
            return 0;
        }
    }
    wr32(m, obj + 0x50u, 0xD5A04u);
    sub_0CBF7A(m, obj);
    return 0;
}
