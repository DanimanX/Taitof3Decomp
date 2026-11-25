#include "obj_DD6FC.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0CF7FE: activate object once +0x3A enters [-0x40, 0x14F]. */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x14F)
        return;
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) & (uint8_t)~0x80u));
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
}

/* ROM 0x0CFA86: load +0x1C, +0x10, +0x68 from record A1. */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
}

/* ROM 0x0D07B4: aim at nearest in-range player and maybe trigger +0x23. */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t off = rd32(m, obj + 0x68u);
    int16_t sx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, off));
    if (sx < 0x28 || sx > 0x127)
        return;
    int16_t sy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, off + 2u));
    if (sy < 0x10 || sy > 0xDF)
        return;

    uint16_t min_d = rd16(m, a1 + 0x08u);
    uint16_t max_d = rd16(m, a1 + 0x0Au);
    uint16_t ang_lo = rd16(m, a1 + 0x0Cu);
    uint16_t ang_hi = rd16(m, a1 + 0x0Eu);

    int16_t d4 = -1, d7 = -1;
    uint16_t d2_abs = 0, d3_abs = 0, d5_abs = 0, d6_abs = 0;

    for (int p = 0; p < 2; p++) {
        uint32_t pa = (p == 0) ? 0x0041B190u : 0x0041B21Au;
        if ((int16_t)rd16(m, pa + 0x4Cu) < 0)
            continue;
        int16_t dy_s = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, pa + 0x18u)
                                 + rd16(m, off + 2u) - 0x0F);
        uint16_t ay = (uint16_t)(dy_s < 0 ? -dy_s : dy_s);
        if (ay > max_d)
            continue;
        int16_t dx_s = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, pa + 0x14u)
                                 + rd16(m, off) - 0x17);
        uint16_t ax;
        if (dx_s < 0) {
            ax = (uint16_t)(-dx_s);
            if (ax > max_d)
                continue;
            if (ax < min_d && ay < min_d)
                continue;
        } else {
            ax = (uint16_t)dx_s;
            if (ax > max_d)
                continue;
            if ((int16_t)ax < (int16_t)(min_d - 0x1Eu) && ay < min_d)
                continue;
        }
        uint16_t ang = Obj_Atan256(m, dx_s, dy_s);
        int ok = (ang_lo > ang_hi) ? (ang >= ang_lo || ang <= ang_hi)
                                   : (ang >= ang_lo && ang <= ang_hi);
        if (!ok)
            continue;
        if (p == 0) {
            d4 = (int16_t)ang;
            d2_abs = ax;
            d3_abs = ay;
        } else {
            d7 = (int16_t)ang;
            d5_abs = ax;
            d6_abs = ay;
        }
    }

    int16_t chosen = d7;
    if (d4 < 0) {
        if (d7 < 0)
            return;
    } else if (d7 >= 0) {
        uint16_t m1 = (d2_abs <= d3_abs) ? d2_abs : d3_abs;
        uint16_t m2 = (d5_abs <= d6_abs) ? d5_abs : d6_abs;
        if (m2 >= m1)
            chosen = d4;
    } else {
        chosen = d4;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    uint16_t diff_bonus = rd16(m, 0x000D0D02u + (uint32_t)((rd8(m, F3_A5 + 0x87Fu) >> 3) & 0x1Fu) * 2u);
    int16_t thresh = (int16_t)(rd16(m, a1 + 0x10u) + diff_bonus);
    int16_t rng = (int16_t)Obj_Next4Way(m);
    if (thresh > rng)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0DD6FC */
int Obj_DD6FC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x00000320u);
            wr32(m, obj + 0x10u, 0x0005C454u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 7u));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x00000320u);
        goto die_spawn;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    die_spawn:
        {
            uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
            if ((int32_t)child >= 0) {
                wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x24u));
                wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 8u));
                wr16(m, child + 0x58u, 0u);
                wr16(m, child + 0x60u, 0u);
            }
            Obj_StartDying(m, obj);
            return 0;
        }
    }

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0:
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    case 1:
        if (rd8(m, obj + 0x23u) == 0u) {
            sub_0D07B4(m, obj, 0x000DD9B6u);
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
        if (rd8(m, obj + 0x23u) == 0u) {
            Obj_StartBehaviour(m, obj, 0x000DDA30u);
            wr8(m, obj + 0x23u, 0x0Au);
            wr8(m, obj + 0x06u, 2u);
        }
        break;
    case 2:
        Obj_StepScript(m, obj);
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
        if (rd8(m, obj + 0x23u) == 0u)
            wr8(m, obj + 0x06u, 3u);
        break;
    case 3:
        if (rd8(m, obj + 0x23u) == 0u) {
            sub_0D07B4(m, obj, 0x000DD9EEu);
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
        if (rd8(m, obj + 0x23u) == 0u) {
            Obj_StartBehaviour(m, obj, 0x000DDA54u);
            wr8(m, obj + 0x06u, 4u);
        }
        break;
    case 4:
        Obj_StepScript(m, obj);
        if (rd16(m, obj + 0x4Cu) == 0u) {
            sub_0CFA86(m, obj, 0x000DDA26u);
            wr8(m, obj + 0x23u, (uint8_t)(0x1Eu - (rd8(m, F3_A5 + 0x87Fu) >> 4)));
            wr8(m, obj + 0x06u, 5u);
        } else if ((int16_t)rd16(m, obj + 0x68u) >= 0) {
            static const uint16_t angles[3] = { 0x00E8u, 0x00D4u, 0x00C1u };
            uint32_t sx = rd32(m, obj + 0x14u) + 0x00270000u;
            uint32_t sy = rd32(m, obj + 0x18u) + 0x00030000u;
            for (int i = 0; i < 3; i++) {
                uint32_t p = Obj_Spawn(m, obj, 0x000D9224u, 9u, 2u, 0u);
                if ((int32_t)p < 0)
                    break;
                wr32(m, p + 0x14u, sx);
                wr32(m, p + 0x18u, sy);
                wr16(m, p + 0x58u, angles[i]);
                wr16(m, p + 0x10u, 0x001Bu);
            }
        }
        break;
    case 5:
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
        if (rd8(m, obj + 0x23u) == 0u)
            wr8(m, obj + 0x06u, 1u);
        break;
    default:
        break;
    }

    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
