#include "obj_186482.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x17BD1A: apply screen/boss shake offset from 0x17BD48/0x17BD4A to +0x14/+0x18 */
static void sub_17BD1A(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x4B3Bu) & 0x02u)
        return;
    uint16_t d0 = (uint16_t)(((rd16(m, F3_A5 + 0x0878u) >> 3) + rd8(m, obj + 0x22u)) & 7u);
    int16_t dx = (int16_t)rd16(m, 0x0017BD48u + (uint32_t)d0 * 4u);
    int16_t dy = (int16_t)rd16(m, 0x0017BD4Au + (uint32_t)d0 * 4u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + dy));
}

/* ROM 0x17B9BA: spawn hit spark when +0x35 != 0 and random(5) == 0 */
static void sub_17B9BA(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    if (Obj_RandRange(m, 5u) != 0u)
        return;
    (void)Obj_Spawn(m, obj, 0x0017A122u, 0x0Bu, 1u, 0u);
}

/* ROM 0x186482: boss body segment tick */
int Obj_186482_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {
        Obj_Free(m, obj);
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u)
        wr8(m, obj + 0x5Eu, rd8(m, obj + 0x0Cu));

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        wr32(m, F3_A5 + 0x0906u, p2 + 4u);
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    uint32_t a1 = rd32(m, obj + 0x10u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(rd16(m, a1 + 2u) << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(rd16(m, a1) << 3)));

    sub_17BD1A(m, obj);

    uint32_t a4 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x58u, rd32(m, a4 + 0x58u));
    wr16(m, obj + 0x1Eu, rd16(m, a4 + 0x1Eu));

    Obj_BossMapScreenPos(m, obj);
    sub_17B9BA(m, obj);

    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    wr8(m, obj + 0x35u, 0u);
    Obj_DrawSprite(m, obj);

    wr32(m, obj + 0x00u, 0x00186482u);
    return 0;
}
