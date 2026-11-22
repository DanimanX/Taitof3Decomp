#include "obj_D76F4.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in)
{
    uint16_t d0 = d0_in, d1 = d1_in, d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t t = d0;
        d0 = d1;
        d1 = t;
    }

    d1 = (uint16_t)(d1 << 6);
    uint32_t a0 = 0x00026E40u + (uint32_t)(int32_t)(int16_t)d1;
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0));
    d0 = (uint16_t)(d0 + rd16(m, 0x00006492u + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x0CBDDE: owner through the +0x36 link (one indirection for kind 1) */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x0CBE34: colour override / blue flash */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0x00u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
        return;
    }
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
}

/* ROM 0x0D76F4 */
int Obj_D76F4_Tick(const f3mem_t *m, uint32_t obj)
{
    if (!((int32_t)rd32(m, obj + 0x24u) > 0)) {
        /* 0x0D795E: score to the owner, then the death burst */
        uint32_t a1 = sub_0CBDDE(m, obj);
        uint32_t d0 = rd16(m, obj + 0x4Cu);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
        return -1;                               /* 0x0D796E sound/spawn path */
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        return -1;                               /* 0x0D796E sound/spawn path */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            wr32(m, F3_A5 + 0x1D50u, rd32(m, F3_A5 + 0x1D50u) + rd16(m, obj + 0x4Cu));
            return -1;                           /* 0x0CBD20 jmp 0x0CBB2C */
        }
    }

    /* 0x0D772A: turn toward the top-centre point, clamped by (+0x60)/2 */
    {
        uint16_t d0 = (uint16_t)(0x0090u - rd16(m, obj + 0x14u));
        uint16_t d1 = (uint16_t)(0x004Eu - rd16(m, obj + 0x18u));
        d0 = sub_006422(m, d0, d1);
        d0 = (uint16_t)(d0 << 8);
        d0 = (uint16_t)(d0 - rd16(m, obj + 0x20u));
        d1 = (uint16_t)((int16_t)rd16(m, obj + 0x60u) >> 1);
        if ((int16_t)d0 > (int16_t)d1) {
            d0 = d1;
        } else if ((int16_t)d0 < -(int16_t)d1) {
            d0 = (uint16_t)(0u - d1);
        }
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0));
    }

    wr16(m, obj + 0x42u, (uint16_t)((int16_t)rd16(m, obj + 0x62u) >> 8));
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    {
        /* 0x0D7776 sub_0CBDA4 falls through to 0x0CBDAE: two lists */
        uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
        a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
    }
    sub_0CBE34(m, obj);
    Obj_DrawSprite(m, obj);

    /* 0x0D7788: aim angle <= 0x0A steps the animation at 0x0CBD24 with a
       table operand that the flow-following listing does not include */
    if ((int16_t)rd16(m, obj + 0x42u) <= 0x0A)
        return -1;

    /* 0x0D779E */
    wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x62u)
                                    - (uint16_t)((int16_t)rd16(m, obj + 0x60u) >> 2)));
    if ((int16_t)rd16(m, obj + 0x62u) > 0) {
        wr32(m, obj + 0x00u, 0x000D76F4u);
        return 0;
    }
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x00u, 0x000D77C4u);
    return 0;
}
