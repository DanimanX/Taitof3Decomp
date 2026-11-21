#include "obj_F30B4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CEC4C */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;

    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t idx = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 0x05u)
        idx = (uint16_t)(idx + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)idx * 4u);
    wr32(m, obj + 0x0Cu, a2);

    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));
}

/* ROM 0x0CEE2C: colour index from A0+0x4E via a byte table */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = rd8(m, obj + 0x4Eu);
    /* 0x0CEE34: move.b (0xCEE40,PC,D0w),D0b */
    uint8_t c = rd8(m, 0x000CEE40u + d0);
    wr8(m, obj + 0x1Du, c);
    wr8(m, obj + 0x4Fu, c);
}

/* ROM 0x0CF1D8 */
static void sub_0CF1D8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x44u, a1);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u)); /* bset.b #1 */
}

/* ROM 0x0D0050 */
static void sub_0D0050(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, a1 + 0x28u, 0u);
    (void)0;
}

/* ROM 0x0F30B4 */
int Obj_F30B4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0F30B4: lea (0x298,PC),A1 */
    sub_0CEC4C(m, obj, (uint32_t)(0x0F30B6u + 0x298u));
    sub_0CEE2C(m, obj);
    sub_0CF1D8(m, obj, (uint32_t)(0x0F30C6u + 0x2C0u));
    sub_0D0050(m, obj);

    /* 0x0F30D4 */
    wr32(m, obj + 0x68u, 0x000F34C8u);
    wr16(m, obj + 0x58u, 0x010Eu);
    wr16(m, obj + 0x60u, 0x0012u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* bset.b #2 */
    wr32(m, obj + 0x00u, 0x000F30F6u);
    return 0;
}
