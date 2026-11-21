#include "obj_CABF4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

#define ATAN2_TBL        0x00026E40u
#define ATAN2_QUAD_TBL   0x00006492u
#define ENEMY_LIST_BASE  0x0041A988u
#define ENEMY_LIST_END   (F3_A5 + 0x08FEu)
#define SHOT_LIST_CURSOR (F3_A5 + 0x0902u)
#define FRAME_COUNTER    (F3_A5 + 0x0878u)
#define SCR_OFFS_X       (F3_A5 + 0x1D20u)
#define SCR_OFFS_Y       (F3_A5 + 0x1D24u)
#define PLAYER_CTRL_FLG  (F3_A5 + 0x1D36u)
#define BOUNDS_X_ADDR    0x000CADB2u
#define BOUNDS_Y_ADDR    0x000CADB6u
#define SPR_TBL_BASE     0x000CADBAu
#define SPR_TBL_SHOT     0x000CAE3Au

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = ATAN2_TBL;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 2u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 4u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t val = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    int16_t d3 = 0x20;
    for (int i = 15; i >= 0; i--) {
        if ((val & (uint16_t)(1u << i)) != 0u) {
            d3 = (int16_t)(0x10 + (15 - i));
            break;
        }
    }

    d3 = (int16_t)(d3 - 0x1A);
    if (d3 < 0) {
        uint16_t shift = (uint16_t)(0u - (uint16_t)d3);
        d0 = (uint16_t)(d0 >> (shift & 63u));
        d1 = (uint16_t)(d1 >> (shift & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 = (uint32_t)(a0 + (int32_t)(int16_t)d1);
    uint8_t b = rd8(m, (uint32_t)(a0 + (int32_t)(int16_t)d0));
    d0 = (uint16_t)((d0 & 0xFF00u) | b);
    d0 = (uint16_t)(d0 + rd16(m, (uint32_t)(ATAN2_QUAD_TBL + (int32_t)(int16_t)d2)));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x0CABF4 */
int Obj_CABF4_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    uint32_t a1 = rd32(m, obj + 0x08u);
    if ((int16_t)rd16(m, a1 + 0x4Cu) < 0)
        goto lab_0cabec;
    if ((rd8(m, a1 + 0x65u) & 0x01u) != 0u)
        goto lab_0cabec;
    if ((int16_t)rd16(m, a1 + 0x54u) < 0x0C)
        goto lab_0cabec;

    uint16_t d0 = 0u;
    a1 = rd32(m, obj + 0x08u);
    a1 = rd32(m, a1 + 0x44u);
    if ((rd8(m, a1) & 0x10u) == 0u) {
        a1 = 0u;
        uint32_t a2 = ENEMY_LIST_BASE;
        while (a2 != rd32(m, ENEMY_LIST_END)) {
            uint32_t a3 = rd32(m, a2);
            a2 += 4u;
            if ((rd8(m, a3 + 0x34u) & 0x08u) != 0u)
                continue;
            a1 = a3;
            if ((rd8(m, a3 + 0x34u) & 0x04u) != 0u)
                break;
        }
        if (a1 == 0u) {
            d0 = 0u;
        } else {
            uint16_t d2 = (uint16_t)(rd16(m, a1 + 0x2Eu) + rd16(m, a1 + 0x2Cu));
            d2 >>= 1;
            d0 = (uint16_t)(rd16(m, a1 + 0x14u) + d2);
            d0 = (uint16_t)(d0 - rd16(m, obj + 0x14u));
            d0 = (uint16_t)(d0 - 8u);

            d2 = (uint16_t)(rd16(m, a1 + 0x32u) + rd16(m, a1 + 0x30u));
            d2 >>= 1;
            uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x18u) + d2);
            d1 = (uint16_t)(d1 - rd16(m, obj + 0x18u));
            d1 = (uint16_t)(d1 - 8u);

            d0 = sub_006422(m, d0, d1);
            d0 = (uint16_t)(d0 << 8);
        }
    }

    uint16_t cur_ang = rd16(m, obj + 0x20u);
    uint16_t diff = (uint16_t)(d0 - cur_ang);
    uint16_t step = (uint16_t)((diff >> 5) | ((diff & 0x8000u) ? 0xF800u : 0u));
    wr16(m, obj + 0x20u, (uint16_t)(cur_ang + step));

    a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x0Cu) + rd16(m, a1 + 0x14u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x0Eu) + rd16(m, a1 + 0x18u)));

    if (((uint16_t)(rd16(m, a1 + 0x22u) + rd16(m, FRAME_COUNTER)) & 1u) == 0u) {
        uint16_t idx = (uint16_t)((uint16_t)(rd16(m, obj + 0x20u) + 0x0400u) >> 11);
        wr32(m, obj + 0x10u, rd32(m, SPR_TBL_BASE + (uint32_t)idx * 4u));
        uint16_t orig_x = rd16(m, obj + 0x14u);
        uint16_t orig_y = rd16(m, obj + 0x18u);
        wr16(m, obj + 0x14u, (uint16_t)(orig_x + rd16(m, SCR_OFFS_X)));
        wr16(m, obj + 0x18u, (uint16_t)(orig_y + rd16(m, SCR_OFFS_Y)));
        Obj_DrawSprite(m, obj);
        wr16(m, obj + 0x18u, orig_y);
        wr16(m, obj + 0x14u, orig_x);
    }

    if (rd8(m, obj + 0x06u) == 0u) {
        if ((rd8(m, PLAYER_CTRL_FLG) & 0x04u) != 0u)
            goto lab_0cabec;
        a1 = rd32(m, obj + 0x08u);
        a1 = rd32(m, a1 + 0x44u);
        if ((rd8(m, a1) & 0x10u) != 0u)
            goto lab_0cabec;
        wr8(m, obj + 0x06u, 0xFFu);
        wr16(m, obj + 0x42u, 0x0018u);
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto lab_0cabe0;

    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t xlo = (int16_t)rd16(m, BOUNDS_X_ADDR);
    int16_t xhi = (int16_t)rd16(m, BOUNDS_X_ADDR + 2u);
    if (x < xlo || x > xhi)
        goto lab_0cabe0;

    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    int16_t ylo = (int16_t)rd16(m, BOUNDS_Y_ADDR);
    int16_t yhi = (int16_t)rd16(m, BOUNDS_Y_ADDR + 2u);
    if (y < ylo || y > yhi)
        goto lab_0cabe0;

    if (Terrain_TestPoint(m, (int16_t)(uint16_t)((uint16_t)x + 8u),
                             (int16_t)(uint16_t)((uint16_t)y + 8u)) != 0)
        goto lab_0cabe0;

    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 0x0040u));
    uint16_t idx = (uint16_t)((uint16_t)(rd16(m, obj + 0x20u) + 0x0400u) >> 11);
    wr32(m, obj + 0x10u, rd32(m, SPR_TBL_SHOT + (uint32_t)idx * 4u));
    Obj_DrawSprite(m, obj);

    uint32_t p = rd32(m, SHOT_LIST_CURSOR);
    wr32(m, p, obj);
    wr32(m, SHOT_LIST_CURSOR, p + 4u);
    goto lab_0cabec;

lab_0cabe0:
    wr32(m, obj + 0x24u, 0x00010000u);
    wr8(m, obj + 0x06u, 0x00u);

lab_0cabec:
    wr32(m, obj, 0x000CABF4u);
    return 0;
}
