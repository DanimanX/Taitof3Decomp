#include "obj_D740E.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0D7D48: vertical bob, direction in bit 0 of +0x5C */
static void sub_0D7D48(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0w = rd16(m, obj + 0x5Cu);
    if (d0w & 1u) {
        uint16_t v = (uint16_t)(rd16(m, obj + 0x5Cu) + 0x0222u);
        wr16(m, obj + 0x5Cu, v);
        if ((int16_t)v >= 0x4000)
            d0w ^= 1u;
    } else {
        uint16_t v = (uint16_t)(rd16(m, obj + 0x5Cu) - 0x0222u);
        wr16(m, obj + 0x5Cu, v);
        if ((int16_t)v <= -0x4000)
            d0w ^= 1u;
    }
    /* bfins D0,(0x5C,A0){15:1} */
    wr16(m, obj + 0x5Cu, (uint16_t)((rd16(m, obj + 0x5Cu) & 0xFFFEu) | (d0w & 1u)));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)(int32_t)(int16_t)d0w);
}

/* ROM 0x0D740E */
int Obj_D740E_Tick(const f3mem_t *m, uint32_t obj)
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

    /* 0x0D7444 */
    sub_0D7D48(m, obj);
    sub_0CBDA4(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {
            wr32(m, obj + 0x00u, 0x000D740Eu);
            return 0;
        }
    }
    wr16(m, obj + 0x62u, 0u);
    wr16(m, obj + 0x20u, (uint16_t)((Rng_NextMem(m) & 0x3FFFu) - 0x2000u));
    wr32(m, obj + 0x00u, 0x000D747Eu);
    return 0;

lab_d752e:
    play_sound(m, (uint16_t)rd8(m, obj + 0x65u));
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, sub_13E4D0(m, obj, 2u));
    wr32(m, obj + 0x00u, 0x000D7570u);
    return 0;
}
