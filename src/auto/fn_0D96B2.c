/* ROM 0x0D96B2: torpedo/missile enemy tick (A0 = object).
 * Handles debug-kill check (A5+0x1D58 bit 1, X in [-10, 0x11C] -> 0x0CBB2C),
 * HP/bomb death (adjusting X +0x4F if state==2, spawning 0x0DBDA2 + 0x0CEE74),
 * cooldown +0x23, velocity integration (+0x50/+0x54 with scroll A5+0x4224/0x4228),
 * state machine (0x0D97A2: align horizontally with P1/P2 and launch left/right),
 * bounds/terrain collision (0x0C6632), script step (0x0D0690), hazard (0x0D0DCC),
 * and sprite draw (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

int Fn_0D96B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x0D96B2: debug kill check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);                          /* 0x0D96CA -> 0x0CBB2C */
            return 0;
        }
    }

    /* 0x0D96D0: HP <= 0 or bomb flag */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 ||
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        if (rd8(m, obj + 0x06u) == 2u)                        /* 0x0D9770 */
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x004Fu));
        r->d[0] = Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u); /* 0x0D9780..0x0D9792 */
        Obj_StartDying(m, obj);                               /* 0x0D979C -> 0x0CEE74 */
        return 0;
    }

    /* 0x0D96E2: cooldown +0x23 */
    if (rd8(m, obj + 0x23u) != 0u)
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));

    /* 0x0D96EE: integrate X/Y velocity */
    {
        uint32_t d0 = rd32(m, obj + 0x50u);
        if (rd8(m, obj + 0x06u) == 0u)
            d0 -= rd32(m, F3_A5 + 0x4224u);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);

        d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);
        r->d[0] = d0;
    }

    /* 0x0D970E..0x0D9714: jump table 0x0D97A2 */
    switch (rd8(m, obj + 0x06u)) {
    case 0:                                                   /* 0x0D97AE */
        if (rd8(m, obj + 0x23u) != 0u)
            break;
        if ((int16_t)rd16(m, obj + 0x14u) < 0)
            break;
        {
            uint32_t target = 0x0041B190u;
            int aligned = 0;
            if ((int16_t)rd16(m, target + 0x4Cu) >= 0) {
                int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, target + 0x18u) - 6);
                if (dy < 0) dy = (int16_t)(-dy);
                if ((uint16_t)dy <= 3u)
                    aligned = 1;
            }
            if (!aligned) {
                target = 0x0041B21Au;
                if ((int16_t)rd16(m, target + 0x4Cu) < 0)
                    break;
                int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, target + 0x18u) - 6);
                if (dy < 0) dy = (int16_t)(-dy);
                if ((uint16_t)dy > 3u)
                    break;
            }
            /* 0x0D980E */
            wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) + 3u));
            int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, target + 0x14u) - 0x10);
            uint32_t vx, vy;
            if (dx >= 0) {
                Obj_StartBehaviour(m, obj, 0x000D9902u);      /* 0x0D9822..0x0D9826 */
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 5u));
                wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
                wr16(m, obj + 0x2Cu, 0x0008u);
                wr16(m, obj + 0x2Eu, 0x000Fu);
                wr16(m, obj + 0x30u, 0x0004u);
                wr16(m, obj + 0x32u, 0x0009u);
                sub_0D2D44(m, 0x00B4u, rd16(m, obj + 0x60u), &vx, &vy);
                wr32(m, obj + 0x50u, vx);
                wr32(m, obj + 0x54u, vy);
                wr8(m, obj + 0x06u, 1u);
            } else {
                Obj_StartBehaviour(m, obj, 0x000D994Au);      /* 0x0D986A..0x0D986E */
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x4Cu));
                wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
                wr16(m, obj + 0x2Cu, 0x0050u);
                wr16(m, obj + 0x2Eu, 0x0057u);
                wr16(m, obj + 0x30u, 0x0004u);
                wr16(m, obj + 0x32u, 0x0009u);
                sub_0D2D44(m, 0x0000u, rd16(m, obj + 0x60u), &vx, &vy);
                wr32(m, obj + 0x50u, vx);
                wr32(m, obj + 0x54u, vy);
                wr8(m, obj + 0x06u, 2u);
            }
        }
        break;
    case 1:
    case 2:
        break;
    default:
        return -1;
    }

    /* 0x0D971C..0x0D976E */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x < -0x70 || x > 0x01AF) {
            Obj_StartDying(m, obj);                           /* 0x0CEE74 */
            return 0;
        }
    }
    {
        uint8_t state = rd8(m, obj + 0x06u);
        int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, 0x000D98B2u + (uint32_t)state * 4u));
        int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, 0x000D98B2u + (uint32_t)state * 4u + 2u));
        if (Terrain_TestPoint(m, tx, ty) != 0) {              /* 0x0D974E -> 0x0C6632 */
            Obj_StartDying(m, obj);                           /* 0x0CEE74 */
            return 0;
        }
    }

    Obj_StepScript(m, obj);                                   /* 0x0D975C -> 0x0D0690 */
    Coll_AddHazard(m, obj);                                   /* 0x0D9762 -> 0x0D0DCC */
    Obj_DrawSprite(m, obj);                                   /* 0x0D9768 -> 0x001282 */
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);
    r->d[0] = 0x0000FFFFu;
    r->d[1] = 0x0001FFFFu;
    return 0;
}
