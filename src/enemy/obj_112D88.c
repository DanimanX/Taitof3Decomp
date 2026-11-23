#include "obj_112D88.h"
#include "../objects/obj_shared.h"
#include "obj_112C3C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 -> 0x0039D8 */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x13E4D0: D0 = (y*8)<<24 | (x*8)<<16 | D0w */
static uint32_t sub_13E4D0(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint32_t d1 = (uint32_t)(uint16_t)(rd16(m, a1) << 3);
    d0 = (d0 & 0xFF00FFFFu) | ((d1 & 0xFFu) << 16);
    d1 = (uint32_t)(uint16_t)(rd16(m, a1 + 2u) << 3);
    d0 = (d0 & 0x00FFFFFFu) | ((d1 & 0xFFu) << 24);
    return d0;
}

/* ROM 0x0CBDA4 (+0x0CBDAE): append obj to lists A5+0x8FE and A5+0x906 */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0CBDDE: A1 = owner to credit */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x0CBE34: hit flash */

/* ROM 0x0017D6 */
static void sub_0017D6(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d1, uint32_t d2, uint32_t d6)
{
    uint32_t a2 = (F3_A5 - 0x7F50u) + d6;
    int32_t div = (int32_t)d1;
    uint16_t d7 = (uint16_t)((uint16_t)d2 - 1u);
    for (;;) {
        uint32_t c0 = rd32(m, a0);
        a0 += 4u;
        uint32_t c1 = rd32(m, a1);
        a1 += 4u;
        for (int k = 0; k < 3; k++) {
            unsigned sh = 16u - 8u * (unsigned)k;
            uint32_t s = ((c0 >> sh) & 0xFFu) << 8;
            wr16(m, a2 + 2u * (uint32_t)k, (uint16_t)s);
            uint32_t e = ((c1 >> sh) & 0xFFu) << 8;
            int32_t step = (int32_t)(e - s) / div;
            wr16(m, a2 + 6u + 2u * (uint32_t)k, (uint16_t)(uint32_t)step);
        }
        a2 += 0x0Cu;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            break;
    }
}

/* ROM 0x001792 (via 0x00183C): claim a palette-ramp slot and build the ramp */
static void sub_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint32_t d0, uint32_t d1, uint32_t d2)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0u;
    uint16_t d7 = 0x000Fu;
    for (;;) {
        if (rd16(m, a3) == 0u) {
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0)
                return;
            wr16(m, a3, (uint16_t)d2);
            if ((uint16_t)d2 == 0u)
                return;
            wr16(m, a3 + 4u, (uint16_t)d0);
            sub_0017D6(m, a0, a1, d1, d2, d6);
            return;
        }
        d6 = (d6 & 0xFFFF0000u) | (uint16_t)((uint16_t)d6 + 0x0180u);
        a3 += 6u;
        d7 = (uint16_t)(d7 - 1u);
        if (d7 == 0xFFFFu)
            return;
    }
}

/* ROM 0x112D88 */
int Obj_112D88_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t f = rd8(m, a1 + 0x50u);
    if (f & 0x10u) {
        /* 0x113162 */
        play_sound(m, 0x0035u);
        Obj_Spawn(m, obj, 0x0013E4EEu, 3u, 2u, sub_13E4D0(m, obj, 3u));
        wr32(m, obj + 0x00u, 0x001131A0u);
        return 0;
    }
    if (f & 0x20u) {
        wr32(m, obj + 0x00u, 0x001131A0u);  /* 0x113198 */
        return 0;
    }

    int32_t d0 = (int32_t)(rd32(m, obj + 0x56u) - rd32(m, obj + 0x24u));
    if (d0 != 0) {
        uint8_t k = rd8(m, rd32(m, obj + 0x36u) + 0x22u);
        if (k != 0u && k != 4u && k != 5u) {
            d0 >>= 1;                       /* asr.l #1 */
            wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) + (uint32_t)d0);
        }
    }

    /* 0x112DD4 */
    uint32_t hp = rd32(m, obj + 0x24u);
    wr32(m, obj + 0x56u, hp);
    if ((int32_t)hp > 0) {
        sub_0CBDA4(m, obj);
        Obj_Animate(m, obj, 0x001131B8u);  /* lea (0x3d2,PC) at 0x112DE4 */
        Obj_113400_FollowParent(m, obj);
        Obj_BlinkColour(m, obj);
        Obj_DrawSprite(m, obj);
        wr32(m, obj + 0x00u, 0x00112D88u);
        return 0;
    }

    /* 0x112E04 */
    uint32_t own = sub_0CBDDE(m, obj);
    wr32(m, own + 0x48u, rd32(m, own + 0x48u) + 0x2710u);
    Obj_Spawn(m, obj, 0x00117B44u, 3u, 2u, 0x00002710u);
    uint32_t sc = rd32(m, obj + 0x0Cu);
    uint32_t src = rd32(m, sc);
    uint16_t dst = rd16(m, sc + 8u);
    uint16_t cnt = rd16(m, sc + 10u);
    sub_001792(m, src, 0x001136EAu, dst, 0x1Eu, cnt);
    a1 = rd32(m, obj + 0x08u);
    wr8(m, a1 + 0x50u, (uint8_t)(rd8(m, a1 + 0x50u) | 0x01u));
    play_sound(m, 0x0033u);
    Obj_Spawn(m, obj, 0x0013E4EEu, 3u, 2u, sub_13E4D0(m, obj, 3u));
    wr8(m, obj + 0x35u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x20u));
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    wr16(m, obj + 0x42u, 0x0005u);
    wr16(m, obj + 0x20u, 0x00C0u);
    wr32(m, obj + 0x00u, 0x00112EC0u);     /* 0x112EB8 */
    return 0;
}
