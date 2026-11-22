#include "obj_D56C6.h"
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


/* ROM 0x0CBF7A */
static void sub_0CBF7A(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x28740u);
    wr32(m, obj, 0xCBF8Au);
}


/* ROM 0x0D56C6 */
int Obj_D56C6_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    sub_0CBE66(m, obj, 0xD5680u);
    sub_0CEE2C(m, obj);
    wr32(m, obj + 0x14u, rd32(m, rd32(m, obj + 0x08u) + 0x14u));
    wr16(m, obj + 0x60u, sub_0CBEB0(m, 0x100u));
    wr16(m, obj + 0x62u, (uint16_t)(sub_0CBEB0(m, 0x14u) << 6));
    wr16(m, obj + 0x20u, 0xE000u);
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x5Eu, 0u);
    uint16_t d = rd16(m, obj + 0x22u);
    if (d != 0u) {
        d = (uint16_t)((int32_t)(int16_t)d * 10);
        wr16(m, obj + 0x06u, d);
        if (d != 0u) {
            wr32(m, obj, 0xD572Cu);
            return 0;
        }
    }
    uint32_t a1 = rd32(m, rd32(m, obj + 0x08u) + 0x50u);
    int gt = (int16_t)rd16(m, a1 + 0x18u) > 0x6C;
    wr8(m, obj + 0x64u, gt ? 0x02u : 0x00u);
    wr32(m, obj + 0x18u, 0u);
    wr16(m, obj + 0x18u, gt ? 0xFFF0u : 0x00E8u);
    wr32(m, obj + 0x50u, 0xD576Cu);
    sub_0CBF7A(m, obj);
    return 0;
}
