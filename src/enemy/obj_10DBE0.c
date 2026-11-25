#include "obj_10DBE0.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CFA86: install {flip.w, spr.l, aux.l} record at A1 */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 0x02u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 0x06u));
}

/* ROM 0x10DE20: bomb/destroy trigger when A5+0x1D58 bit 1 is set and onscreen */
static void sub_10DE20(const f3mem_t *m, uint32_t obj)
{
    uint8_t f7 = rd8(m, obj + 0x07u);
    uint32_t d0 = rd32(m, 0x0010E0B0u + (uint32_t)f7 * 4u);
    Obj_RelayTimer(m, d0);

    int16_t d7 = (int16_t)((uint16_t)rd8(m, obj + 0x4Eu) + 1u);
    uint32_t d6 = rd32(m, obj + 0x3Au);
    uint32_t d5 = rd32(m, obj + 0x3Eu);
    uint16_t h = (uint16_t)(rd16(m, obj + 0x42u) + 2u);
    d5 += ((uint32_t)h << 16);
    uint32_t a4 = 0x0010E15Au;

    for (; d7 >= 0; d7--) {
        uint32_t child = Obj_Spawn(m, obj, 0x0010E2CEu, 9u, 2u, 0u);
        if ((int32_t)child < 0)
            break;
        wr32(m, child + 0x14u, d6);
        wr32(m, child + 0x18u, d5);
        wr32(m, child + 0x10u, a4);
        d6 += 0x00100000u;
        a4 = (d7 == 1) ? 0x0010E16Eu : 0x0010E164u;
    }
    wr8(m, obj + 0x06u, 5u);
}

/* Common preamble of states 1..4: sync +0x14/+0x18, check bomb trigger */
static int state_preamble(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x48u)));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -10 && x <= 0x011C) {
            sub_10DE20(m, obj);
            return 1;
        }
    }
    return 0;
}

/* ROM 0x10DD74: state 1 - idle, wait for player X proximity */
static void state_1(const f3mem_t *m, uint32_t obj)
{
    if (state_preamble(m, obj))
        return;
    Obj_0D0DC2(m, obj);
    if (rd8(m, obj + 0x07u) == 3u) {
        wr8(m, obj + 0x06u, 4u);
        return;
    }

    uint16_t d1 = (uint16_t)(rd16(m, obj + 0x48u) + 0x001Eu);
    static const uint32_t players[2] = { 0x0041B190u, 0x0041B21Au };
    for (int i = 0; i < 2; i++) {
        uint32_t pl = players[i];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;
        int16_t d0 = (int16_t)((int16_t)rd16(m, obj + 0x14u) - (int16_t)rd16(m, pl + 0x14u) - 0x17);
        if (d0 < 0)
            d0 = (int16_t)-d0;
        if ((uint16_t)d0 <= d1) {
            wr8(m, obj + 0x22u, 1u);
            wr8(m, obj + 0x06u, 2u);
            return;
        }
    }
}

/* ROM 0x10DEA8: state 2 - extend downward until terrain hit or Y > 0xE8 */
static void state_2(const f3mem_t *m, uint32_t obj)
{
    if (state_preamble(m, obj))
        return;
    Obj_0D0DC2(m, obj);
    if (rd8(m, obj + 0x07u) == 3u) {
        wr8(m, obj + 0x06u, 4u);
        return;
    }

    uint8_t c22 = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
    wr8(m, obj + 0x22u, c22);
    if (c22 == 0u) {
        wr8(m, obj + 0x22u, 5u);
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x003D));
    }

    int16_t d7 = (int16_t)((uint16_t)rd8(m, obj + 0x4Eu) + 1u);
    int16_t d6 = (int16_t)(rd16(m, obj + 0x3Au) + 8u);
    int16_t d5 = (int16_t)(rd16(m, obj + 0x3Eu) + rd16(m, obj + 0x42u) + 0x1Eu);
    uint8_t d4 = 0u;

    if (d5 <= 0x00E8) {
        for (; d7 >= 0; d7--) {
            d4 = (uint8_t)(d4 + (uint8_t)Terrain_TestPoint(m, d6, d5));
            d6 = (int16_t)(d6 + 0x10);
        }
        if (d4 == 0u) {
            wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 4u));
            return;
        }
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0025));
    }

    wr8(m, obj + 0x22u, 1u);
    wr8(m, obj + 0x23u, 0x14u);
    wr8(m, obj + 0x06u, 3u);
}

/* ROM 0x10DF76: state 3 - pause then retract upward */
static void state_3(const f3mem_t *m, uint32_t obj)
{
    if (state_preamble(m, obj))
        return;
    Obj_0D0DC2(m, obj);
    if (rd8(m, obj + 0x07u) == 3u) {
        wr8(m, obj + 0x06u, 4u);
        return;
    }

    uint8_t c23 = rd8(m, obj + 0x23u);
    if (c23 != 0u) {
        c23--;
        wr8(m, obj + 0x23u, c23);
        if (c23 != 0u)
            return;
    }

    uint8_t c22 = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
    wr8(m, obj + 0x22u, c22);
    if (c22 == 0u) {
        wr8(m, obj + 0x22u, 0x0Au);
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x003D));
    }

    int16_t h = (int16_t)(rd16(m, obj + 0x42u) - 2u);
    if (h >= 0) {
        wr16(m, obj + 0x42u, (uint16_t)h);
    } else {
        wr16(m, obj + 0x42u, 0u);
        wr8(m, obj + 0x06u, 1u);
    }
}

/* ROM 0x10DFFC: state 4 - fall with gravity until terrain hit or Y > 0x10E */
static void state_4(const f3mem_t *m, uint32_t obj)
{
    if (state_preamble(m, obj))
        return;
    Obj_0D0DC2(m, obj);

    int16_t d7 = (int16_t)((uint16_t)rd8(m, obj + 0x4Eu) + 1u);
    int16_t d6 = (int16_t)(rd16(m, obj + 0x3Au) + 8u);
    int16_t d5 = (int16_t)(rd16(m, obj + 0x3Eu) + rd16(m, obj + 0x42u) + 0x1Eu);
    uint8_t d4 = 0u;

    if (d5 > 0x010E) {
        wr8(m, obj + 0x06u, 6u);
        return;
    }

    for (; d7 >= 0; d7--) {
        d4 = (uint8_t)(d4 + (uint8_t)Terrain_TestPoint(m, d6, d5));
        d6 = (int16_t)(d6 + 0x10);
    }

    if (d4 == 0u) {
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, obj + 0x54u)));
        wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u);
        return;
    }

    if (rd8(m, obj + 0x20u) == 0u) {
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0025));
        wr8(m, obj + 0x20u, 1u);
    }
}

/* ROM 0x10DBE0 */
int Obj_10DBE0_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    if ((int16_t)rd16(m, obj + 0x3Au) < (int16_t)rd16(m, obj + 0x4Cu) ||
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

    uint8_t st = rd8(m, obj + 0x06u);
    switch (st) {
    case 1: state_1(m, obj); break;
    case 2: state_2(m, obj); break;
    case 3: state_3(m, obj); break;
    case 4: state_4(m, obj); break;
    case 5:
    case 6: break;
    default: return -1;
    }

    if (rd8(m, obj + 0x06u) < 4u) {
        int16_t y_stop = (int16_t)(rd16(m, obj + 0x3Eu) - 8u);
        int16_t y_off = (int16_t)(rd16(m, obj + 0x42u) - 8u);

        uint32_t left = rd32(m, obj + 0x60u);
        wr32(m, left + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, left + 0x18u, rd32(m, obj + 0x3Eu));
        wr16(m, left + 0x18u, (uint16_t)(rd16(m, left + 0x18u) + (uint16_t)y_off));
        do {
            Obj_DrawSprite(m, left);
            wr16(m, left + 0x18u, (uint16_t)(rd16(m, left + 0x18u) - 0x10u));
        } while (y_stop <= (int16_t)rd16(m, left + 0x18u));

        uint32_t right = rd32(m, obj + 0x64u);
        wr32(m, right + 0x14u, rd32(m, obj + 0x3Au));
        wr16(m, right + 0x14u, (uint16_t)(rd16(m, right + 0x14u) + rd16(m, obj + 0x44u) - 0x10u));
        wr32(m, right + 0x18u, rd32(m, obj + 0x3Eu));
        wr16(m, right + 0x18u, (uint16_t)(rd16(m, right + 0x18u) + (uint16_t)y_off));
        do {
            Obj_DrawSprite(m, right);
            wr16(m, right + 0x18u, (uint16_t)(rd16(m, right + 0x18u) - 0x10u));
        } while (y_stop <= (int16_t)rd16(m, right + 0x18u));
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x3Au));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x3Eu));
    sub_0CFA86(m, obj, 0x0010E11Eu);
    for (int16_t d7 = (int16_t)((uint16_t)rd8(m, obj + 0x4Eu) + 1u); d7 >= 0; d7--) {
        Obj_DrawSprite(m, obj);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
        sub_0CFA86(m, obj, (d7 == 1) ? 0x0010E132u : 0x0010E128u);
    }

    if (rd8(m, obj + 0x06u) < 5u) {
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x42u) + 2u));
        sub_0CFA86(m, obj, 0x0010E13Cu);
        for (int16_t d7 = (int16_t)((uint16_t)rd8(m, obj + 0x4Eu) + 1u); d7 >= 0; d7--) {
            Obj_DrawSprite(m, obj);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
            sub_0CFA86(m, obj, (d7 == 1) ? 0x0010E150u : 0x0010E146u);
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x42u) + 2u));
    return 0;
}
