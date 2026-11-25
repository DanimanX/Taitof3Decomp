#include "obj_F7800.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables {0x0D2D64, 0x0D2ECCu}
 * indexed by D0w and scaled by D1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x0F7800 */
int Obj_F7800_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    uint8_t p_state = rd8(m, p + 0x06u);

    if (rd8(m, obj + 0x06u) == 0u) {
        if (p_state == 0u)
            Obj_StartDying(m, obj);
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        uint16_t bit = rd16(m, obj + 0x0Cu);
        wr8(m, p + 0x4Eu, (uint8_t)(rd8(m, p + 0x4Eu) & (uint8_t)~(1u << (bit & 7u))));
        Obj_SetFrame(m, obj, 0x000F7B40u);
        Obj_RelayTimer(m, 0x190u);
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        Obj_ExplodeTick(m, obj);
        return 0;
    }

    int goto_die = 0;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x190u);
        goto_die = 1;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        goto_die = 1;
    } else if (p_state == 0x09u && rd8(m, obj + 0x06u) <= 0x06u) {
        goto_die = 1;
    }

    if (goto_die) {
        uint32_t expl = Obj_Spawn(m, obj, 0x000DC586u, 3u, 2u, 0u);
        if ((int32_t)expl >= 0) {
            wr32(m, expl + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, expl + 0x14u, (uint16_t)(rd16(m, expl + 0x14u) + 0x0010u));
            wr32(m, expl + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, expl + 0x18u, (uint16_t)(rd16(m, expl + 0x18u) + 0x0010u));
            wr16(m, expl + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, expl + 0x60u, rd16(m, obj + 0x60u));
        }
        uint16_t bit = rd16(m, obj + 0x0Cu);
        wr8(m, p + 0x4Eu, (uint8_t)(rd8(m, p + 0x4Eu) & (uint8_t)~(1u << (bit & 7u))));
        wr8(m, obj + 0x06u, 0u);
        return 0;
    }

    switch (rd8(m, obj + 0x06u)) {
    case 1u: {
        uint16_t idx = rd16(m, obj + 0x0Cu);
        if ((uint8_t)idx < rd8(m, p + 0x22u)) {
            wr8(m, obj + 0x23u, 0x04u);
            wr8(m, obj + 0x06u, 0x03u);
        } else if (rd8(m, p + 0x06u) >= 0x04u) {
            wr8(m, obj + 0x06u, 0x02u);
        }
        break;
    }
    case 2u: {
        uint16_t idx = rd16(m, obj + 0x0Cu);
        if ((uint8_t)idx < rd8(m, p + 0x22u)) {
            wr8(m, obj + 0x23u, 0x06u);
            wr8(m, obj + 0x06u, 0x06u);
        }
        break;
    }
    case 3u:
    case 4u: {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u) {
            wr8(m, obj + 0x23u, 0x04u);
            wr8(m, obj + 0x06u, (uint8_t)(rd8(m, obj + 0x06u) + 1u));
        }
        break;
    }
    case 5u:
    case 6u: {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u) {
            wr8(m, obj + 0x23u, 0x06u);
            wr8(m, obj + 0x06u, (uint8_t)(rd8(m, obj + 0x06u) + 1u));
        }
        break;
    }
    case 7u: {
        int32_t v = (int32_t)rd32(m, obj + 0x60u) - (int32_t)rd32(m, obj + 0x64u);
        wr32(m, obj + 0x60u, (uint32_t)v);
        if (v < 0) {
            wr32(m, obj + 0x60u, 0u);
            wr8(m, obj + 0x06u, 0x08u);
        } else {
            uint32_t vx, vy;
            sub_0D2D44(m, 0x005Au, rd16(m, obj + 0x60u), &vx, &vy);
            wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);
        }
        break;
    }
    case 8u: {
        wr32(m, obj + 0x60u, rd32(m, obj + 0x60u) + rd32(m, obj + 0x64u));
        if (rd16(m, obj + 0x60u) > 0x0032u)
            wr16(m, obj + 0x60u, 0x0032u);
        uint32_t vx, vy;
        sub_0D2D44(m, 0x010Eu, rd16(m, obj + 0x60u), &vx, &vy);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);
        int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + 0x0010u);
        int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + 0x001Cu);
        if (Terrain_TestPoint(m, tx, ty) != 0) {
            wr8(m, obj + 0x23u, 0x06u);
            wr8(m, obj + 0x06u, 0x09u);
        }
        break;
    }
    case 9u: {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u) {
            wr8(m, obj + 0x23u, 0x0Fu);
            wr8(m, obj + 0x06u, 0x0Au);
        }
        break;
    }
    case 10u: {
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t);
        if (t == 0u) {
            uint16_t idx = rd16(m, obj + 0x0Cu);
            if (rd8(m, obj + 0x4Fu) != 0u)
                idx += 4u;
            uint32_t fn = rd32(m, 0x000F7B68u + (uint32_t)idx * 4u);
            uint32_t child = Obj_Spawn(m, obj, fn, 5u, 2u, 0u);
            if ((int32_t)child >= 0) {
                wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            }
            wr8(m, obj + 0x06u, 0x0Bu);
        }
        break;
    }
    default:
        return -1;
    }

    uint8_t s = rd8(m, obj + 0x06u);
    if (s == 0x0Bu) {
        uint16_t bit = rd16(m, obj + 0x0Cu);
        wr8(m, p + 0x4Eu, (uint8_t)(rd8(m, p + 0x4Eu) & (uint8_t)~(1u << (bit & 7u))));
        wr8(m, obj + 0x06u, 0u);
        return 0;
    }
    if (s >= 0x07u) {
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
        if (s >= 0x09u)
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    }

    if ((int16_t)rd16(m, obj + 0x18u) > 0x00FA) {
        uint16_t bit = rd16(m, obj + 0x0Cu);
        wr8(m, p + 0x4Eu, (uint8_t)(rd8(m, p + 0x4Eu) & (uint8_t)~(1u << (bit & 7u))));
        wr8(m, obj + 0x06u, 0u);
        return 0;
    }

    Obj_SetFrame(m, obj, rd32(m, 0x000F7AF2u + (uint32_t)s * 4u));
    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    return 0;
}
