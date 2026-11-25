#include "obj_1865A0.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1 = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = (uint32_t)d0w * (uint32_t)d1;
    uint16_t mod = (uint16_t)((prod << 8) >> 16);
    if (mod == 0u)
        mod = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % mod);
}

/* ROM 0x185A0A: part callback used by 0x186972 */
static int sub_185A0A(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, F3_A5 + 0x4B38u) == 1u)
        return 0;
    if (rd16(m, obj + 0x59u) != 0u)
        return 0;
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        uint16_t idx = rd16(m, 0x001869A2u);
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, 0x001869A4u);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
    }
    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x08u) != 0u &&
        rd32(m, obj + 0x10u) == 0x00031FB0u &&
        sub_0CC2BC(m, 0x0014u) == 0u) {
        (void)Obj_Spawn(m, obj, 0x0017D912u, 9u, 2u, 0xFBF800F0u);
    }
    return 0;
}

/* ROM 0x1865A0: boss part tick (record at +0x0C) */
int Obj_1865A0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {
        Obj_Free(m, obj);
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        return -1; /* dynamic jump (a3+0x24) on death */

    uint32_t cb = rd32(m, a3 + 0x14u);
    if (cb == 0x00185A0Au) {
        if (sub_185A0A(m, obj) < 0)
            return -1;
    } else if (cb != 0u) {
        return -1;
    }

    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
    wr16(m, obj + 0x1Eu, rd16(m, a4 + 0x1Eu));
    wr32(m, obj + 0x58u, rd32(m, a4 + 0x58u));

    if (rd8(m, obj + 0x1Du) == 0x06u)
        wr8(m, obj + 0x1Du, 0x08u);
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, 0x06u);
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    wr8(m, obj + 0x35u, 0u);

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    uint32_t a1 = rd32(m, obj + 0x10u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(rd16(m, a1 + 2u) << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(rd16(m, a1) << 3)));

    Obj_BossMapScreenPos(m, obj);
    Obj_DrawSprite(m, obj);

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        wr32(m, F3_A5 + 0x0906u, p2 + 4u);
    }

    wr32(m, obj + 0x00u, 0x001865A0u);
    return 0;
}
