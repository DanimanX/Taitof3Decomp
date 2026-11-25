#include "obj_E54EE.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D0C8C */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint16_t idx = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)idx * 2u);
}

/* ROM 0x0D0090 */
static void sub_0D0090(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w)
{
    int16_t off = (int16_t)rd16(m, (uint32_t)(0x000D00B4 + (int32_t)(int16_t)d0w * 2));
    uint32_t ptr = rd32(m, (uint32_t)((int32_t)a1 + (int32_t)off));
    wr32(m, obj + 0x48u, ptr);
    wr16(m, obj + 0x4Cu, rd16(m, ptr));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, ptr + 2u));
    wr32(m, obj + 0x10u, rd32(m, ptr + 4u));
    wr32(m, obj + 0x68u, rd32(m, ptr + 8u));
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

/* ROM 0x0D0BBC: step facing angle (+0x08) toward target angle (+0x42). */
static void sub_0D0BBC(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t target = rd16(m, obj + 0x42u);
    int16_t diff = (int16_t)(target - rd16(m, obj + 0x08u));
    if (diff < 0)
        diff = (int16_t)(diff + 0x0168);

    uint16_t step_w = rd16(m, a1 + 0x04u);
    uint32_t step_l = rd32(m, a1 + 0x04u);
    if ((uint16_t)diff > 0x00B4u) {
        uint16_t inv = (uint16_t)(0x0168 - diff);
        if (inv <= step_w) {
            wr16(m, obj + 0x08u, target);
            return;
        }
        int32_t cur = (int32_t)(rd32(m, obj + 0x08u) - step_l);
        if (cur < 0)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) + 0x0168u));
        else
            wr32(m, obj + 0x08u, (uint32_t)cur);
    } else {
        if ((uint16_t)diff <= step_w) {
            wr16(m, obj + 0x08u, target);
            return;
        }
        wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) + step_l);
        if (rd16(m, obj + 0x08u) >= 0x0168u)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) - 0x0168u));
    }
}

/* ROM 0x0E54EE */
int Obj_E54EE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x0Cu);
    if (rd8(m, parent + 0x06u) == 0u)
        goto spawn_dead;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        wr8(m, parent + 0x06u, 0x03u);
        Score_OnHit(m, obj, 0x0000012Cu);
        goto spawn_dead;
    }

    wr32(m, obj + 0x3Au, rd32(m, parent + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, parent + 0x18u));

    uint32_t off_addr = 0x000E5756u + (uint32_t)rd8(m, parent + 0x23u) * 4u;
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, off_addr)));
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + rd16(m, off_addr + 2u)));

    Obj_FlagOffscreen(m, obj);
    if (rd8(m, obj + 0x07u) & 0x08u) {
        wr8(m, parent + 0x06u, 0x03u);
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    uint8_t state = rd8(m, obj + 0x06u);
    switch (state) {
    case 0u:
        Obj_ArmOnScreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x06u, 0x01u);
        }
        break;
    case 1u:
        if (rd8(m, obj + 0x23u) == 0u) {
            sub_0D07B4(m, obj, 0x000E571Eu);
            sub_0D0BBC(m, obj, 0x000E571Eu);
            Obj_0CFA7C(m, obj, 0x000E5766u, rd16(m, obj + 0x08u));
        }
        if (rd8(m, obj + 0x23u) != 0u) {
            uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, c);
            if (c == 0u) {
                sub_0D0090(m, obj, 0x000E5806u, rd16(m, obj + 0x08u));
                wr8(m, obj + 0x06u, 0x02u);
            }
        }
        break;
    case 2u:
        Obj_StepScript(m, obj);
        if (rd16(m, obj + 0x4Cu) != 0u) {
            int16_t d1w = (int16_t)rd16(m, obj + 0x68u);
            if (d1w >= 0) {
                uint32_t child = Obj_Spawn(m, obj, 0x000D9224u, 9u, 2u, 0u);
                if ((int32_t)child >= 0) {
                    wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0035u));
                    wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x000Au));
                    wr16(m, child + 0x58u, (uint16_t)d1w);
                    wr16(m, child + 0x10u, 0x001Bu);
                    wr8(m, parent + 0x06u, 0x02u);
                    break;
                }
                Obj_0CFA7C(m, obj, 0x000E5766u, rd16(m, obj + 0x08u));
                wr8(m, obj + 0x23u, 0x0Au);
                wr8(m, obj + 0x06u, 0x03u);
            }
        } else {
            Obj_0CFA7C(m, obj, 0x000E5766u, rd16(m, obj + 0x08u));
            wr8(m, obj + 0x23u, 0x0Au);
            wr8(m, obj + 0x06u, 0x03u);
        }
        wr8(m, parent + 0x06u, 0x01u);
        break;
    case 3u: {
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c);
        if (c == 0u) {
            wr8(m, obj + 0x06u, 0x01u);
        } else {
            sub_0D07B4(m, obj, 0x000E571Eu);
            wr8(m, obj + 0x23u, c);
            sub_0D0BBC(m, obj, 0x000E571Eu);
            Obj_0CFA7C(m, obj, 0x000E5766u, rd16(m, obj + 0x08u));
        }
        break;
    }
    default:
        return -1;
    }

    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    Obj_DrawSprite(m, parent);
    return 0;

spawn_dead:
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
        if ((int32_t)child >= 0) {
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x002Eu));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x000Au));
            wr16(m, child + 0x58u, 0u);
            wr16(m, child + 0x60u, 0u);
        }
    }
    Obj_StartDying(m, obj);
    return 0;
}
