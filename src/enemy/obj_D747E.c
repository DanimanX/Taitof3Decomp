#include "obj_D747E.h"
#include "../objects/obj_shared.h"
#include "obj_D7570.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

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
    if (rd8(m, obj + 0x35u) != 2u && rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
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
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
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
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

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

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1w = (uint16_t)(d1 >> 16);
    if (d1w == 0u)
        d1w = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % d1w);
}

/* ROM 0x0CC33A: angle from obj to the player at A5+0x1D28 (0x80 if none) */
static uint16_t sub_0CC33A(const f3mem_t *m, uint32_t obj, uint16_t d0w, uint16_t d1w)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
    if (a1 == 0u)
        return 0x0080u;
    d0w = (uint16_t)(0u - d0w);
    uint16_t d2w = (uint16_t)ASR16((uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu)), 1u);
    d0w = (uint16_t)(d0w + d2w + rd16(m, a1 + 0x14u) - rd16(m, obj + 0x14u));
    d1w = (uint16_t)(0u - d1w);
    d2w = (uint16_t)ASR16((uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u)), 1u);
    d1w = (uint16_t)(d1w + d2w + rd16(m, a1 + 0x18u) - rd16(m, obj + 0x18u));
    return sub_006422(m, d0w, d1w);
}

/* ROM 0x0D759C: occasionally fire an aimed bullet (0xD901A) */
static void sub_0D759C(const f3mem_t *m, uint32_t obj)
{
    if (sub_0CC1E0(m, obj) != 0u)
        return;
    if (sub_0CC24A(m, obj) != 0u)
        return;
    if (sub_0CC2BC(m, 0x0004u) != 0u)
        return;
    uint16_t d0w = (uint16_t)(sub_0CC33A(m, obj, 0x0002u, 0x0024u) << 8);
    uint32_t d1 = 0x02240000u | d0w;
    uint16_t r = (uint16_t)((Rng_NextMem(m) & 0x1F00u) - 0x1000u);
    d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 + r);
    Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, d1);
}

/* ROM 0x0D747E */
int Obj_D747E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x0D751E */
        uint32_t a1 = sub_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto lab_d752e;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto lab_d752e;

    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x0CBD12 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    /* 0x0D74B4 */
    {
        uint32_t a1 = 0x000D7614u;
        Obj_AnimFrameStep(m, obj, a1);
        uint16_t d0w = rd16(m, obj + 0x3Au);   /* final offset (+0x3A), as in ROM */
        if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0w) <= 0) {
            uint8_t b = rd8(m, obj + 0x1Cu);
            wr8(m, obj + 0x1Cu, (uint8_t)(b | 0x01u));
            if ((b & 0x01u) == 0u)
                wr32(m, obj + 0x10u, 0x0006423Cu);
        }
    }

    /* 0x0D74DA */
    wr16(m, obj + 0x42u, (uint16_t)ASR16(rd16(m, obj + 0x62u), 8u));
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x62u) + (uint16_t)ASR16(rd16(m, obj + 0x60u), 1u)));
    sub_0D759C(m, obj);
    sub_0CBDA4(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    if ((int16_t)rd16(m, obj + 0x14u) > 0x140)
        return Obj_D7570_Tick(m, obj);   /* bgt.w 0x0D7570 */
    wr32(m, obj + 0x00u, 0x000D747Eu);
    return 0;

lab_d752e:
    play_sound(m, (uint16_t)rd8(m, obj + 0x65u));
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, sub_13E4D0(m, obj, 2u));
    wr32(m, obj + 0x00u, 0x000D7570u);
    return 0;
}
