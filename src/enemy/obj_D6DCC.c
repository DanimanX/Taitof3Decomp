#include "obj_D6DCC.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0CBEB0 */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0w)
{
    int16_t d1w = (int16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    int32_t prod = (int32_t)(int16_t)d0w * (int32_t)d1w;
    return (uint16_t)((uint32_t)(prod << 4) >> 16);
}

/* ROM 0x0CBEC2 */
static void sub_0CBEC2(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int16_t d1 = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d0 * 2u);
    int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + (uint32_t)d0 * 2u);
    wr32(m, obj + 0x54u, (uint32_t)((int32_t)d1 * (int32_t)cos_v));
    wr32(m, obj + 0x58u, (uint32_t)((int32_t)d1 * (int32_t)sin_v));
}

/* ROM 0x0CC24A */
static uint8_t sub_0CC24A(const f3mem_t *m, uint32_t obj)
{
    int16_t cx = (int16_t)(((int16_t)rd16(m, obj + 0x2Cu) + (int16_t)rd16(m, obj + 0x2Eu)) >> 1);
    cx = (int16_t)(cx + (int16_t)rd16(m, obj + 0x14u));
    if (cx < 16 || cx > 304)
        return 0xFFu;
    int16_t cy = (int16_t)(((int16_t)rd16(m, obj + 0x30u) + (int16_t)rd16(m, obj + 0x32u)) >> 1);
    cy = (int16_t)(cy + (int16_t)rd16(m, obj + 0x18u));
    if (cy < 16 || cy > 216)
        return 0xFFu;
    return 0u;
}

/* ROM 0x0D704E..0x0D709E */
static void die_D6DCC(const f3mem_t *m, uint32_t obj, int add_score)
{
    if (add_score) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }
    Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u)));
    uint32_t arg = Obj_13E4D0(m, obj, 2u);
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg);
    wr32(m, obj + 0x00u, 0x000D70A0u);
}

/* ROM 0x0D7020..0x0D704A */
static void jump_D6DCC(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x4Fu) == 0u) {
        wr16(m, obj + 0x20u, 0xB000u);
        wr16(m, obj + 0x42u, 0x0020u);
    } else {
        wr16(m, obj + 0x20u, 0xA800u);
        wr16(m, obj + 0x42u, 0x0024u);
        wr16(m, obj + 0x06u, 1u);
    }
    sub_0CBEC2(m, obj);
    wr32(m, obj + 0x00u, 0x000D6BD8u);
}

/* ROM 0x0D6DCC */
int Obj_D6DCC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        die_D6DCC(m, obj, 1);
        return 0;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        die_D6DCC(m, obj, 0);
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < -120 || x > 500) {
        Obj_Free(m, obj);
        return 0;
    }
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (y < -80 || y > 232) {
        Obj_Free(m, obj);
        return 0;
    }

    Obj_0D0DC2(m, obj);
    Obj_SubScrollVel(m, obj);

    uint8_t fl = rd8(m, obj + 0x64u);
    wr8(m, obj + 0x1Cu, fl);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    Obj_AnimFrameStep(m, obj, 0x000D7112u);
    int16_t off = (int16_t)rd16(m, obj + 0x3Au);
    int16_t dur = (int16_t)rd16(m, 0x000D7112u + (uint32_t)(int32_t)off);
    if (dur > 0) {
        wr32(m, obj + 0x00u, 0x000D6DCCu);
        return 0;
    }
    if (dur == 0) {
        wr8(m, obj + 0x64u, (uint8_t)(fl ^ 0x01u));
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
        wr32(m, obj + 0x00u, 0x000D6DCCu);
        return 0;
    }

    /* 0x0D6E66 */
    if ((uint8_t)Obj_0D70AE(m, obj, 0x0028, 0x0030) != 0u) {
        wr16(m, obj + 0x06u, 2u);
        jump_D6DCC(m, obj);
        return 0;
    }

    wr16(m, obj + 0x3Au, 0xFFFEu);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr16(m, obj + 0x06u, 0x0050u);

    /* 0x0D6F0C */
    Obj_AnimFrameStep(m, obj, 0x000D7148u);
    int16_t off2 = (int16_t)rd16(m, obj + 0x3Au);
    int16_t d2w = (int16_t)rd16(m, 0x000D7148u + (uint32_t)(int32_t)off2);
    if (d2w <= 0) {
        int16_t t22 = (int16_t)(rd16(m, obj + 0x22u) - 1u);
        wr16(m, obj + 0x22u, (uint16_t)t22);
        if (t22 > 0) {
            if ((int16_t)rd16(m, obj + 0x14u) <= 0x0014) {
                wr16(m, obj + 0x06u, 2u);
                jump_D6DCC(m, obj);
                return 0;
            }
        } else {
            int16_t d0w = (int16_t)sub_0CBEB0(m, 0x0018u);
            wr16(m, obj + 0x22u, (uint16_t)(0x001E - d0w));
            d2w = (int16_t)(d2w + 6);
            if (d2w > 0)
                wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
            else
                wr16(m, obj + 0x3Au, 0u);

            uint32_t d1 = rd32(m, 0x000D71C0u + (uint32_t)(int32_t)d2w * 4u);
            int16_t ox = (int8_t)(d1 >> 24);
            int16_t oy = (int8_t)(d1 >> 16);
            uint16_t d1w = (uint16_t)d1;
            if ((fl & 0x01u) != 0u) {
                d1w = (uint16_t)(-(int16_t)(d1w + 0x4000) - 0x4000);
                ox = (int16_t)(-ox);
            }
            if ((fl & 0x02u) != 0u) {
                d1w = (uint16_t)(-(int16_t)d1w);
                oy = (int16_t)(-oy);
            }
            d1 = (d1 & 0xFFFF0000u) | d1w;
            if (sub_0CC24A(m, obj) != 0u) {
                wr16(m, obj + 0x06u, 2u);
                jump_D6DCC(m, obj);
                return 0;
            }
            ox = (int16_t)(ox + 0x28);
            oy = (int16_t)(oy + 0x20);
            d1 = (d1 & 0x0000FFFFu) | (((uint32_t)(uint8_t)ox) << 24) | (((uint32_t)(uint8_t)oy) << 16);
            Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, d1);
            if (rd8(m, obj + 0x4Fu) != 0u) {
                uint32_t d1a = (d1 & 0xFFFF0000u) | (uint16_t)(d1 + 0x0800u);
                Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, d1a);
                uint32_t d1b = (d1a & 0xFFFF0000u) | (uint16_t)(d1a - 0x1000u);
                Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, d1b);
            }
        }
    }

    uint16_t old_cnt = rd16(m, obj + 0x06u);
    wr16(m, obj + 0x06u, (uint16_t)(old_cnt - 1u));
    if (old_cnt > 1u) {
        wr32(m, obj + 0x00u, 0x000D6E98u);
        return 0;
    }
    wr16(m, obj + 0x06u, 2u);
    jump_D6DCC(m, obj);
    return 0;
}
