#include "obj_FEF76.h"
#include "obj_E8B72.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t d0 = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D07B4: aim at nearest live player within bounds/angle window. */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t p68 = rd32(m, obj + 0x68u);
    int16_t cx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, p68));
    int16_t cy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, p68 + 2u));
    if (cx < 0x28 || cx > 0x127 || cy < 0x10 || cy > 0xDF)
        return;

    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    int16_t best_ang[2] = { -1, -1 };
    uint16_t max_dx[2] = { 0, 0 };
    uint16_t max_dy[2] = { 0, 0 };

    uint16_t r_min = rd16(m, a1 + 0x08u);
    uint16_t r_max = rd16(m, a1 + 0x0Au);
    uint16_t a_min = rd16(m, a1 + 0x0Cu);
    uint16_t a_max = rd16(m, a1 + 0x0Eu);

    for (int i = 0; i < 2; i++) {
        uint32_t pl = players[i];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;
        int16_t sdy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u) +
                                rd16(m, p68 + 2u) - 0x0F);
        uint16_t ady = (uint16_t)(sdy < 0 ? -sdy : sdy);
        if (ady > r_max)
            continue;
        int16_t sdx = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u) +
                                rd16(m, p68) - 0x17);
        uint16_t adx = (uint16_t)(sdx < 0 ? -sdx : sdx);
        if (adx > r_max)
            continue;
        if (sdx < 0) {
            if (adx < r_min && ady < r_min)
                continue;
        } else {
            if ((int16_t)adx < (int16_t)(r_min - 0x1E) && ady < r_min)
                continue;
        }
        uint16_t ang = Obj_Atan256(m, sdx, sdy);
        if (a_min > a_max) {
            if (ang < a_min && ang > a_max)
                continue;
        } else {
            if (ang < a_min || ang > a_max)
                continue;
        }
        best_ang[i] = (int16_t)ang;
        max_dx[i] = adx;
        max_dy[i] = ady;
    }

    int16_t chosen = best_ang[1];
    if (best_ang[0] >= 0) {
        if (best_ang[1] < 0) {
            chosen = best_ang[0];
        } else {
            uint16_t d0 = (max_dx[0] >= max_dy[0]) ? max_dx[0] : max_dy[0];
            uint16_t d1 = (max_dx[1] >= max_dy[1]) ? max_dx[1] : max_dy[1];
            if (d1 >= d0)
                chosen = best_ang[0];
        }
    } else if (best_ang[1] < 0) {
        return;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    uint16_t thr = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    if ((int16_t)thr > (int16_t)Obj_Next4Way(m))
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

int Obj_FEF76_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x320u);
            wr32(m, obj + 0x10u, 0x000658B8u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0Fu));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 7u));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x320u);
        goto spawn_death_fx;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    spawn_death_fx:
        {
            uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
            if ((int32_t)child >= 0) {
                wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x24u));
                wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x16u));
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
    case 0u:
        Obj_ArmOnScreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
            wr8(m, obj + 0x06u, 0x01u);
        }
        break;
    case 1u:
        sub_0D07B4(m, obj, 0x000FF0F8u);
        if (rd8(m, obj + 0x23u) != 0u) {
            Obj_StartBehaviour(m, obj, 0x000FF13Au);
            wr8(m, obj + 0x06u, 0x02u);
        }
        break;
    case 2u:
        Obj_StepScript(m, obj);
        if (rd16(m, obj + 0x68u) != 0u)
            Obj_Spawn(m, obj, 0x000FF1DAu, 9u, 2u, 0u);
        break;
    }

    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
