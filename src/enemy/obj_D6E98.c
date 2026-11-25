#include "obj_D6E98.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0CBEB0: scale D0w by difficulty (A5+0x87E). */
static int16_t sub_0CBEB0(const f3mem_t *m, int16_t d0w)
{
    int16_t d1 = (int16_t)(((uint16_t)rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    int32_t prod = ((int32_t)d0w * (int32_t)d1) << 4;
    return (int16_t)(uint16_t)((uint32_t)prod >> 16);
}

/* ROM 0x0CBEC2: set +0x54/+0x58 velocity from angle +0x20 and speed +0x42. */
static void sub_0CBEC2(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    wr32(m, obj + 0x54u, (uint32_t)((int32_t)spd * (int32_t)cos_v));
    wr32(m, obj + 0x58u, (uint32_t)((int32_t)spd * (int32_t)sin_v));
}

/* ROM 0x0CC24A: return 1 if hitbox center is off-screen ([16,304]x[16,216]). */
static int sub_0CC24A(const f3mem_t *m, uint32_t obj)
{
    int16_t cx = (int16_t)(((int16_t)rd16(m, obj + 0x2Cu) + (int16_t)rd16(m, obj + 0x2Eu)) >> 1)
                 + (int16_t)rd16(m, obj + 0x14u);
    if (cx < (int16_t)rd16(m, 0x000CC288u) || cx > (int16_t)rd16(m, 0x000CC28Au))
        return 1;
    int16_t cy = (int16_t)(((int16_t)rd16(m, obj + 0x30u) + (int16_t)rd16(m, obj + 0x32u)) >> 1)
                 + (int16_t)rd16(m, obj + 0x18u);
    if (cy < (int16_t)rd16(m, 0x000CC28Cu) || cy > (int16_t)rd16(m, 0x000CC28Eu))
        return 1;
    return 0;
}

/* ROM 0x0D6E98 */
int Obj_D6E98_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die_fx;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    die_fx:
        {
            int16_t cmd = Sound_PreCmdHook(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u));
            Sound_PlayCmd(m, cmd);
            uint32_t arg = Obj_13E4D0(m, obj, 2u);
            Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg);
            wr32(m, obj + 0x00u, 0x000D70A0u);
            return 0;
        }
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (x < (int16_t)rd16(m, 0x000D71DCu) || x > (int16_t)rd16(m, 0x000D71DEu) ||
        y < (int16_t)rd16(m, 0x000D71E0u) || y > (int16_t)rd16(m, 0x000D71E2u)) {
        Obj_Free(m, obj);
        return 0;
    }

    Obj_0D0DC2(m, obj);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x64u));
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    Obj_AnimFrameStep(m, obj, 0x000D7148u);
    int16_t d2 = (int16_t)rd16(m, 0x000D7148u + rd16(m, obj + 0x3Au));
    if (d2 > 0) {
        goto next_cnt;
    }

    int16_t t22 = (int16_t)(rd16(m, obj + 0x22u) - 1u);
    wr16(m, obj + 0x22u, (uint16_t)t22);
    if (t22 <= 0) {
        int16_t wait = (int16_t)(-sub_0CBEB0(m, 0x18) + 0x1E);
        wr16(m, obj + 0x22u, (uint16_t)wait);
        d2 = (int16_t)(d2 + 6);
        if (d2 > 0) {
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
        } else {
            wr16(m, obj + 0x3Au, 0u);
        }

        uint32_t d1 = rd32(m, 0x000D71C0u + (uint32_t)(int32_t)d2 * 4u);
        int16_t dx_off = (int8_t)(d1 >> 24);
        int16_t dy_off = (int8_t)(d1 >> 16);
        uint8_t flip = rd8(m, obj + 0x64u);
        if ((flip & 0x01u) != 0u) {
            uint16_t w = (uint16_t)(-(int16_t)((uint16_t)d1 + 0x4000u) - 0x4000u);
            d1 = (d1 & 0xFFFF0000u) | w;
            dx_off = (int16_t)(-dx_off);
        }
        if ((flip & 0x02u) != 0u) {
            uint16_t w = (uint16_t)(-(int16_t)(uint16_t)d1);
            d1 = (d1 & 0xFFFF0000u) | w;
            dy_off = (int16_t)(-dy_off);
        }
        if (sub_0CC24A(m, obj) != 0)
            goto phase_advance;

        dx_off = (int16_t)(dx_off + 0x28);
        dy_off = (int16_t)(dy_off + 0x20);
        d1 = (d1 & 0x0000FFFFu)
             | (((uint32_t)(uint8_t)dx_off) << 24)
             | (((uint32_t)(uint8_t)dy_off) << 16);
        Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, d1);
        if (rd8(m, obj + 0x4Fu) != 0u) {
            uint32_t d1_a = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 + 0x0800u);
            Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, d1_a);
            uint32_t d1_b = (d1_a & 0xFFFF0000u) | (uint16_t)((uint16_t)d1_a - 0x1000u);
            Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, d1_b);
        }
        goto next_cnt;
    } else {
        if ((int16_t)rd16(m, obj + 0x14u) <= 0x14)
            goto phase_advance;
    }

next_cnt:
    {
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, cnt);
        if (cnt > 0u) {
            wr32(m, obj + 0x00u, 0x000D6E98u);
            return 0;
        }
    }

phase_advance:
    wr16(m, obj + 0x06u, 2u);
    if (rd8(m, obj + 0x4Fu) == 0u) {
        wr16(m, obj + 0x20u, (uint16_t)-0x5000);
        wr16(m, obj + 0x42u, 0x0020u);
    } else {
        wr16(m, obj + 0x20u, (uint16_t)-0x5800);
        wr16(m, obj + 0x42u, 0x0024u);
        wr16(m, obj + 0x06u, 1u);
    }
    sub_0CBEC2(m, obj);
    wr32(m, obj + 0x00u, 0x000D6BD8u);
    return 0;
}
