#include "obj_D67F6.h"
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


/* ROM 0x0D67F6 */
int Obj_D67F6_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    sub_0CBE66(m, obj, 0xD6AD8u);
    uint16_t d0 = (uint16_t)Rng_NextMem(m);
    d0 = (uint16_t)(((d0 & 7u) + 0xC8u) << 8);
    wr16(m, obj + 0x20u, d0);
    wr16(m, obj + 0x42u, 8u);
    sub_0CBEC2(m, obj);
    wr8(m, obj + 0x4Fu, 0u);
    wr32(m, obj + 0x3Au, 0u);
    wr32(m, obj + 0x3Eu, 0xB800u);
    wr16(m, obj + 0x06u, 0x42u);
    wr32(m, obj, 0xD6846u);
    return 0;
}
