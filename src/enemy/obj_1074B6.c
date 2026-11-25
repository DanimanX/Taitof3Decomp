#include "obj_1074B6.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables {0x0D2D64, 0x0D2ECC}
 * indexed by D0w and scaled by D1w (0x0D4F26 multiplier). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);

    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x1075BE: state 0 (detached / falling offscreen). */
static void state0_fall(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x3Eu));

    if ((int16_t)rd16(m, obj + 0x14u) < -0x20 ||
        (int16_t)rd16(m, obj + 0x18u) > 0x00E8) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        return;
    }

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x000036B0u);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x1075FA: state 1 (grounded / walking along terrain). */
static void state1_ground(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) & ~0x10u));
    if (rd8(m, a4 + 0x06u) == 1u)
        return;

    int16_t d6 = (int16_t)(rd16(m, obj + 0x14u) + 0x14u);
    int16_t d7 = (int16_t)(rd16(m, obj + 0x18u) + 0x0Au);

    if (!Terrain_TestPoint(m, d6, d7)) {
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
        if ((int16_t)rd16(m, obj + 0x68u) >= 0)
            Obj_StartBehaviour(m, obj, 0x0010744Eu);
    } else {
        if (rd16(m, obj + 0x68u) != 0u)
            Obj_StartBehaviour(m, obj, 0x0010741Eu);
        wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) | 0x10u));

        if (Terrain_TestPoint(m, (int16_t)(d6 - 1), d7)) {
            if (!Terrain_TestPoint(m, (int16_t)(d6 + 1), d7))
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 1u));
        } else if (Terrain_TestPoint(m, (int16_t)(d6 + 1), d7)) {
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
        }

        if (Terrain_TestPoint(m, d6, (int16_t)(d7 - 3)))
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 2u));
    }

    if (rd8(m, a4 + 0x06u) > 4u && rd8(m, a4 + 0x22u) == 2u) {
        uint16_t d0w = (rd8(m, a4 + 0x06u) == 5u) ? 0x006Eu : 0x0032u;
        uint32_t vx, vy;
        sub_0D2D44(m, d0w, 0x001Eu, &vx, &vy);
        wr32(m, obj + 0x3Au, vx);
        wr32(m, obj + 0x3Eu, vy);
        Obj_StartBehaviour(m, obj, 0x0010744Eu);
        wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) & ~0x10u));
        wr8(m, obj + 0x06u, 2u);
    }
}

/* ROM 0x107700: state 2 (rising jump arc). */
static void state2_jump_up(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    if (rd8(m, a4 + 0x06u) <= 4u) {
        wr8(m, obj + 0x06u, 1u);
        return;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x3Au));
    uint32_t vy = rd32(m, obj + 0x3Eu);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);
    vy += 0x000036B0u;
    wr32(m, obj + 0x3Eu, vy);
    if ((int32_t)vy >= 0)
        wr8(m, obj + 0x06u, 3u);
}

/* ROM 0x10772E: state 3 (falling jump arc until terrain landing). */
static void state3_jump_down(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    if (rd8(m, a4 + 0x06u) <= 4u) {
        wr8(m, obj + 0x06u, 1u);
        return;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x3Eu));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x000036B0u);

    if (Terrain_TestPoint(m,
                          (int16_t)(rd16(m, obj + 0x14u) + 0x14u),
                          (int16_t)(rd16(m, obj + 0x18u) + 0x0Eu))) {
        Obj_StartBehaviour(m, obj, 0x0010741Eu);
        wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) | 0x10u));
        wr8(m, obj + 0x06u, 1u);
    }
}

/* ROM 0x1074B6 */
int Obj_1074B6_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);

    wr8(m, obj + 0x1Du, 0u);
    if (rd8(m, obj + 0x06u) != 0u) {
        if (rd8(m, a4 + 0x06u) == 0u) {
            uint16_t d1w = (uint16_t)((Obj_Next4Way(m) & 0x07u) + 0x1Cu);
            uint16_t d0w = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) + 0x3Au);
            uint32_t vx, vy;
            sub_0D2D44(m, d0w, d1w, &vx, &vy);
            wr32(m, obj + 0x3Au, vx);
            wr32(m, obj + 0x3Eu, vy);
            wr8(m, obj + 0x06u, 0u);
        } else {
            wr8(m, obj + 0x1Du, rd8(m, a4 + 0x1Du));
            if (rd8(m, a4 + 0x35u) == 0u && (int32_t)rd32(m, a4 + 0x24u) > 0) {
                wr8(m, a4 + 0x35u, rd8(m, obj + 0x35u));
                wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
                wr8(m, obj + 0x35u, 0u);
                if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
                    wr32(m, a4 + 0x24u, 0u);
                    wr32(m, obj + 0x24u, 0x001E0000u);
                }
            }
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    switch (rd8(m, obj + 0x06u)) {
    case 0u: state0_fall(m, obj); break;
    case 1u: state1_ground(m, obj, a4); break;
    case 2u: state2_jump_up(m, obj, a4); break;
    case 3u: state3_jump_down(m, obj, a4); break;
    default: break;
    }

    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    if (rd8(m, obj + 0x06u) != 0u) {
        Obj_0D0DC2(m, obj);
        Obj_StepScript(m, obj);
        if (rd8(m, a4 + 0x06u) == 1u) {
            uint16_t ang = Obj_Atan256(m,
                                       (int16_t)(rd16(m, obj + 0x18u) - 0x00A0),
                                       (int16_t)(rd16(m, obj + 0x14u) - 0x0091));
            uint32_t vx, vy;
            sub_0D2D44(m, ang, 7u, &vx, &vy);
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + vx);
            wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);
        }
    }

    return 0;
}
