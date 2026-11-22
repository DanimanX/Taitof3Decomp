#include "obj_11B472.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../collision/collision.h"
#include "../audio/sound_cmd.h"
#include "obj_11B3D2.h"

/* ROM 0x13F238 + 0x39D8: queue a sound command (+0x53 in the remap mode) */
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

/* ROM 0x006422: compass heading (0..0xFF) of the (D0w,D1w) offset */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in)
{
    uint16_t d0 = d0_in, d1 = d1_in;
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
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    uint8_t tbl = rd8(m, 0x026E40u + (uint32_t)(int32_t)(int16_t)d1 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    d0 = (uint16_t)(d0 + rd16(m, 0x006492u + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x0CBDA4 (+0x0CBDAE): append obj to the shot list and the hazard list */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, COLL_LIST1_END);
    wr32(m, a1, obj);
    wr32(m, COLL_LIST1_END, a1 + 4u);
    a1 = rd32(m, COLL_LIST3_END);
    wr32(m, a1, obj);
    wr32(m, COLL_LIST3_END, a1 + 4u);
}

/* ROM 0x0CBDDE: A1 = owner to credit (+0x8 when +0x35 is 1) */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x11B434: re-anchor the orbit and re-arm the fragment life timer */
static void sub_11B434(const f3mem_t *m, uint32_t obj)
{
    uint16_t r = (uint16_t)(Rng_NextMem(m) & 0x3Fu);
    wr16(m, obj + 0x50u, (uint16_t)(r + 0x94u));
    r = (uint16_t)(Rng_NextMem(m) & 0x3Fu);
    wr16(m, obj + 0x52u, (uint16_t)(r + 0x4Cu));
    r = (uint16_t)(Rng_NextMem(m) & 0x3Fu);
    wr16(m, obj + 0x06u, (uint16_t)(r + 0x46u));
    wr32(m, obj + 0x00u, 0x11B472u);
}

/* ROM 0x0011B472: orbiting enemy. It steers +0x20 toward the anchor
 * +0x50/+0x52 by 0x200 a frame, fans 0x11B3D2 fragments after a life timer,
 * and on the head-on path hands itself to the bomb orbit 0x0CBB2C; otherwise
 * it credits the owner (0x0CBDDE), plays sound 0x1C and switches to the
 * follow-up record 0x0CD444[0] (0x0CD3D6 with D0w = 1). */
int Obj_11B472_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto follow_up;

    if (rd8(m, obj + 0x35u) == 0u) {
        sub_0CBDA4(m, obj);
        if ((int16_t)rd16(m, obj + 0x42u) > 7 && (rd16(m, F3_A5 + 0x0878u) & 3u) == 0u)
            wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 1u));

        /* turn +0x20 toward +0x50/+0x52 */
        uint16_t d0 = sub_006422(m, (uint16_t)(rd16(m, obj + 0x50u) - rd16(m, obj + 0x14u)),
                                (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, obj + 0x18u)));
        uint8_t diff = (uint8_t)((uint8_t)d0 - rd8(m, obj + 0x20u));
        if (diff != 0u) {
            if ((int8_t)diff > 0)
                wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0200u));
            else
                wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0200u));
        }

        Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
        /* lea (0xf2,PC),A1 at 0x11B4EA -> 0x11B5D8 */
        Obj_Animate(m, obj, 0x0011B5D8u);

        if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
            int16_t x = (int16_t)rd16(m, obj + 0x14u);
            if (x >= -0x0A && x <= 0x011C) {
                /* jmp 0x0CBB2C: bomb orbit; its continuation is chosen by +0x00 */
                wr8(m, obj + 0x32u, 0x00u);
                wr8(m, obj + 0x34u, 0xFFu);
                wr32(m, obj + 0x00u, 0x000CBB2Cu);
                return 0;
            }
        }

        Obj_DrawSprite(m, obj);
        uint16_t life = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, life);
        if (life != 0u) {
            wr32(m, obj + 0x00u, 0x11B472u);
            return 0;
        }
        if ((int16_t)(uint16_t)(rd16(m, obj + 0x22u) - 1u) < 0) {
            wr32(m, obj + 0x00u, 0x11B472u);
            return 0;
        }
        wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) - 1u));
        Obj_Spawn(m, obj, 0x11B3D2u, 5u, 2u, (uint32_t)rd16(m, obj + 0x22u) << 16);
        Obj_Spawn(m, obj, 0x11B3D2u, 5u, 2u, (uint32_t)rd16(m, obj + 0x20u));
        sub_11B434(m, obj);
        return 0;
    }

    /* 0x11B54C: killed while alive -> two fragments, then credit the owner */
    {
        uint16_t life = (uint16_t)(rd16(m, obj + 0x22u) - 1u);
        wr16(m, obj + 0x22u, life);
        if ((int16_t)life >= 0) {
            Obj_Spawn(m, obj, 0x11B3D2u, 5u, 2u, (uint32_t)life << 16);
            Obj_Spawn(m, obj, 0x11B3D2u, 5u, 2u, 0u);
        }
    }
    /* 0x0CBDDE + 0x48 += +0x4C: credit the owner */
    {
        uint32_t a1 = sub_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }

/* 0x11B5A2: sound 0x1C, +0x20 -> +0x20.b, record 0 of 0x0CD444 (0x0CD3D6)
 * and one drawn frame of it. */
follow_up:
    play_sound(m, 0x1Cu);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    {
        uint32_t a1 = rd32(m, 0x000CD444u);
        uint16_t snd = rd16(m, a1); a1 += 2u;
        if (snd != 0u)
            play_sound(m, snd);
        wr32(m, obj + 0x0Cu, a1);
    }
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CD412u);
    return 0;
}
