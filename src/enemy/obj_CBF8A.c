#include "obj_CBF8A.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../audio/sound_cmd.h"

#define ASR16(v, n) ((v) < 0 ? (int16_t)(uint16_t)(~((uint16_t)(~(uint16_t)(v)) >> (n))) : (int16_t)((uint16_t)(v) >> (n)))
#define ASR32(v, n) ((v) < 0 ? (int32_t)(~((~(uint32_t)(v)) >> (n))) : (int32_t)((uint32_t)(v) >> (n)))

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x00026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 32u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(16u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x001Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0));
    a0 = 0x00006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBCA6 */
static void sub_0CBCA6(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    uint32_t a2 = rd32(m, obj + 0x08u);
    uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);
    uint32_t d1 = d0 * 10u;

    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);

    uint8_t item = rd8(m, obj + 0x4Eu);
    d0 = (d0 & 0xFFFFFF00u) | (uint32_t)item;
    if (item != 0u)
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);

    uint16_t rem = rd16(m, a2 + 0x58u);
    wr16(m, a2 + 0x58u, (uint16_t)(rem - 1u));
    if ((int16_t)rem <= 1) {
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d1);
        Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d1);
    }

    a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x54u, (uint16_t)(rd16(m, a1 + 0x54u) - 1u));
    Obj_ExplodeTick(m, obj);
}

/* ROM 0x0CBDA4 */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);

    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0CBDDE */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x0CBE34 */

/* ROM 0x0CC0F8 */
static void sub_0CC0F8(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x54u, (uint16_t)(rd16(m, a1 + 0x54u) - 1u));
    Obj_Free(m, obj);
}

/* ROM 0x13E4D0 */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
    d0 = (d0 & 0xFF00FFFFu) | (((uint32_t)d1 & 0xFFu) << 16);
    d1 = (uint16_t)(rd16(m, a1 + 2u) << 3);
    d0 = (d0 & 0x00FFFFFFu) | (((uint32_t)d1 & 0xFFu) << 24);
    return d0;
}

/* ROM 0x13F238 */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    return cmd;
}

/* ROM 0x0CBEC2 */
static void sub_0CBEC2(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d0 * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)d0 * 2u);
    wr32(m, obj + 0x54u, (uint32_t)((int32_t)spd * (int32_t)c));
    wr32(m, obj + 0x58u, (uint32_t)((int32_t)spd * (int32_t)s));
}

/* ROM 0x0D55EA */
static void sub_0D55EA(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, 0x000D53C8u);
    wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) & ~0x02u));
}

/* ROM 0x0D55FC */
static void sub_0D55FC(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x000D53E2u;
    Obj_AnimFrameStep(m, obj, a1);
    uint16_t d0 = rd16(m, obj + 0x3Au);   /* final offset (+0x3A), as in ROM */
    if (rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) == 0u) {
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u));
        wr8(m, obj + 0x64u, (uint8_t)(rd8(m, obj + 0x64u) ^ 0x01u));
    }
    wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) & ~0x02u));
}

/* ROM 0x0CC1E0 */
static uint8_t sub_0CC1E0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = F3_A5 + 0x1D2Cu;
    uint16_t d0 = rd16(m, F3_A5 + 0x1D34u);
    for (;;) {
        int lt = (int16_t)d0 < 1;
        d0 = (uint16_t)(d0 - 1u);
        if (lt)
            return 0x00u;

        uint32_t a2 = rd32(m, a1);
        a1 += 4u;

        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x2Eu));
        d1 = (uint16_t)(d1 - rd16(m, a2 + 0x2Cu));
        d1 = (uint16_t)(d1 - rd16(m, a2 + 0x2Eu));
        int16_t s1 = (int16_t)d1;
        d1 = (uint16_t)ASR16(s1, 1u);
        d1 = (uint16_t)(d1 + rd16(m, obj + 0x14u));
        d1 = (uint16_t)(d1 - rd16(m, a2 + 0x14u));
        if ((int16_t)d1 < (int16_t)rd16(m, 0x000CC242u) || (int16_t)d1 > (int16_t)rd16(m, 0x000CC244u))
            continue;

        d1 = (uint16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x32u));
        d1 = (uint16_t)(d1 - rd16(m, a2 + 0x30u));
        d1 = (uint16_t)(d1 - rd16(m, a2 + 0x32u));
        s1 = (int16_t)d1;
        d1 = (uint16_t)ASR16(s1, 1u);
        d1 = (uint16_t)(d1 + rd16(m, obj + 0x18u));
        d1 = (uint16_t)(d1 - rd16(m, a2 + 0x18u));
        if ((int16_t)d1 < (int16_t)rd16(m, 0x000CC246u) || (int16_t)d1 > (int16_t)rd16(m, 0x000CC248u))
            continue;

        return 0xFFu;
    }
}

/* ROM 0x0CC24A */
static uint8_t sub_0CC24A(const f3mem_t *m, uint32_t obj)
{
    int16_t s1 = (int16_t)(uint16_t)(rd16(m, obj + 0x2Cu) + rd16(m, obj + 0x2Eu));
    int16_t d1 = (int16_t)(uint16_t)((uint16_t)ASR16(s1, 1u) + rd16(m, obj + 0x14u));
    if (d1 < (int16_t)rd16(m, 0x000CC288u) || d1 > (int16_t)rd16(m, 0x000CC28Au))
        return 0xFFu;

    s1 = (int16_t)(uint16_t)(rd16(m, obj + 0x30u) + rd16(m, obj + 0x32u));
    d1 = (int16_t)(uint16_t)((uint16_t)ASR16(s1, 1u) + rd16(m, obj + 0x18u));
    if (d1 < (int16_t)rd16(m, 0x000CC28Cu) || d1 > (int16_t)rd16(m, 0x000CC28Eu))
        return 0xFFu;

    return 0x00u;
}

/* ROM 0x0D0C9E */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D42u + (uint32_t)d0 * 2u);
}

/* ROM 0x0CC306 */
static uint8_t sub_0CC306(const f3mem_t *m, uint16_t d1)
{
    uint16_t d0 = sub_0D0C9E(m);
    int16_t sum = (int16_t)(uint16_t)(d1 + d0);
    int16_t lim = ASR16(sum, 1u);
    uint16_t rng = (uint16_t)(Rng_NextMem(m) & 0x00FFu);
    return (lim <= (int16_t)rng) ? 0xFFu : 0x00u;
}

/* ROM 0x0D5620 */
static void sub_0D5620(const f3mem_t *m, uint32_t obj)
{
    if (sub_0CC1E0(m, obj) != 0u)
        return;
    if (sub_0CC24A(m, obj) != 0u)
        return;
    if (sub_0CC306(m, 0x0010u) != 0u)
        return;
    Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, 0x080D6000u);
}

/* ROM 0x0D54DC */
static uint8_t sub_0D54DC(const f3mem_t *m, uint32_t obj)
{
    sub_0D55EA(m, obj);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x62u) >> 6));
    sub_0CBEC2(m, obj);
    if ((int16_t)rd16(m, obj + 0x14u) > 0x64) {
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x5Cu, (uint16_t)(rd16(m, obj + 0x5Cu) + 1u));
    }
    return 0x00u;
}

/* ROM 0x0D550A */
static uint8_t sub_0D550A(const f3mem_t *m, uint32_t obj)
{
    sub_0D55FC(m, obj);
    wr16(m, obj + 0x42u, 0u);
    uint16_t d0 = rd16(m, obj + 0x62u);
    if ((int16_t)d0 > 0)
        wr16(m, obj + 0x42u, (uint16_t)(d0 >> 6));
    sub_0CBEC2(m, obj);

    d0 = (uint16_t)(rd16(m, obj + 0x60u) >> 1);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0));

    d0 = (uint16_t)(rd16(m, obj + 0x60u) - 0x0060u);
    d0 = (uint16_t)(d0 >> 2);
    wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x62u) - d0));

    if ((int16_t)rd16(m, obj + 0x62u) <= -0x180) {
        wr16(m, obj + 0x62u, 0u);
        uint32_t a1 = rd32(m, obj + 0x08u);
        a1 = rd32(m, a1 + 0x50u);
        uint16_t dx = (uint16_t)(rd16(m, a1 + 0x14u) - rd16(m, obj + 0x14u));
        uint16_t dy = (uint16_t)(rd16(m, a1 + 0x18u) - 0x003Cu - rd16(m, obj + 0x18u));
        uint8_t ang = (uint8_t)sub_006422(m, dx, dy);
        ang = (uint8_t)(((uint8_t)(ang - 0x40u) & 0x7Fu) + 0x40u);
        wr16(m, obj + 0x20u, (uint16_t)((uint16_t)ang << 8));
        wr8(m, obj + 0x64u, 0x00u);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x5Cu, (uint16_t)(rd16(m, obj + 0x5Cu) + 1u));
    }
    return 0x00u;
}

/* ROM 0x0D5594 */
static uint8_t sub_0D5594(const f3mem_t *m, uint32_t obj)
{
    sub_0D55EA(m, obj);
    sub_0D5620(m, obj);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x62u) >> 6));
    sub_0CBEC2(m, obj);

    uint16_t v60 = rd16(m, obj + 0x60u);
    uint16_t d0 = (uint16_t)(v60 >> 3);
    uint16_t d1 = (uint16_t)(v60 >> 2);
    if ((int16_t)d1 > (int16_t)rd16(m, obj + 0x42u))
        wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x62u) + d0));

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u))
        return 0xFFu;

    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u))
        return 0xFFu;

    return 0x00u;
}

/* ROM 0x0CBF8A */
int Obj_CBF8A_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);
        uint32_t d1 = d0 * 10u;
        uint32_t a1 = sub_0CBDDE(m, obj);
        uint32_t a2 = rd32(m, obj + 0x08u);

        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);

        int16_t cmd = sub_13F238(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u));
        Sound_PlayCmd(m, cmd);

        d0 = sub_13E4D0(m, obj, 2u);
        d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0);

        uint8_t item = rd8(m, obj + 0x4Eu);
        d0 = (d0 & 0xFFFFFF00u) | (uint32_t)item;
        if (item != 0u)
            Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);

        uint16_t rem = rd16(m, a2 + 0x58u);
        wr16(m, a2 + 0x58u, (uint16_t)(rem - 1u));
        if ((int16_t)rem <= 1) {
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d1);
            Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, d1);
        }

        wr32(m, obj, 0x000CC0F4u);
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        int16_t cmd = sub_13F238(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u));
        Sound_PlayCmd(m, cmd);

        uint32_t d0 = sub_13E4D0(m, obj, 2u);
        Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0);

        wr32(m, obj, 0x000CC05Au);
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            sub_0CBCA6(m, obj);
            return 0;
        }
    }

    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x64u));

    uint32_t tbl = rd32(m, obj + 0x50u);
    int16_t idx = (int16_t)rd16(m, obj + 0x5Cu);
    uint32_t cb = rd32(m, tbl + (uint32_t)(int32_t)idx * 4u);
    uint8_t done;
    switch (cb) {
    case 0x000D54DCu:
        done = sub_0D54DC(m, obj);
        break;
    case 0x000D550Au:
        done = sub_0D550A(m, obj);
        break;
    case 0x000D5594u:
        done = sub_0D5594(m, obj);
        break;
    default:
        /* unimplemented: ROM 0x0CBFD2 */
        return -1;
    }

    if (done != 0u) {
        sub_0CC0F8(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x54u));

    uint32_t dy = rd32(m, obj + 0x58u);
    if ((rd8(m, obj + 0x64u) & 0x02u) != 0u)
        dy = 0u - dy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);

    sub_0CBDA4(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    wr32(m, obj, 0x000CBF8Au);
    return 0;
}
