#include "obj_D7A86.h"
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

/* ROM 0x0D7C74: terrain test at (x+0x10 +/- d0, y+0x26+d1), facing = +0x1C bit 0 */
static uint8_t probe(const f3mem_t *m, uint32_t obj, uint16_t d0, uint16_t d1)
{
    if ((rd8(m, obj + 0x1Cu) & 0x01u) == 0u)
        d0 = (uint16_t)(0u - d0);
    d0 = (uint16_t)(d0 + 0x10u + rd16(m, obj + 0x14u));
    d1 = (uint16_t)(d1 + 0x26u + rd16(m, obj + 0x18u));
    return (uint8_t)Terrain_TestPoint(m, (int16_t)d0, (int16_t)d1);
}

/* ROM 0x0D7C98: ease x speed +0x54 toward +/-d0 (1/4) */
static void ease_x(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, obj + 0x1Cu) & 0x01u) == 0u)
        d0 = 0u - d0;
    d0 -= rd32(m, obj + 0x54u);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)ASR32(d0, 2));
}

/* ROM 0x0D7CB0: ease y speed +0x58 toward d0 (1/8) */
static void ease_y(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    d0 -= rd32(m, obj + 0x58u);
    wr32(m, obj + 0x58u, rd32(m, obj + 0x58u) + (uint32_t)ASR32(d0, 3));
}

/* ROM 0x0D7CBC: scan up/down from the feet for the nearer terrain edge */
static uint32_t scan_vert(const f3mem_t *m, uint32_t obj)
{
    uint16_t d2 = (uint16_t)(rd16(m, obj + 0x14u) + 0x10u);
    uint16_t d3 = (uint16_t)(rd16(m, obj + 0x18u) + 0x26u);
    uint16_t d4 = d3;
    for (;;) {
        d3 = (uint16_t)(d3 - 8u);
        d4 = (uint16_t)(d4 + 8u);
        if ((int16_t)d3 < 0)
            return 0x00030000u;
        if ((uint8_t)Terrain_TestPoint(m, (int16_t)d2, (int16_t)d3) != 0u)
            return 0x00030000u;
        if ((int16_t)d4 > 0xE8)
            return 0xFFFD0000u;
        if ((uint8_t)Terrain_TestPoint(m, (int16_t)d2, (int16_t)d4) != 0u)
            return 0xFFFD0000u;
    }
}

/* ROM 0x0D7A86: hovering walker that follows the terrain and fires */
int Obj_D7A86_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = sub_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto die;
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);   /* 0x0CBD12 */
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x54u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x58u));
    Obj_SubScrollVel(m, obj);
    sub_0CBDA4(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    if (probe(m, obj, 0x18u, 0u)) {
        ease_x(m, obj, 0u);
        ease_y(m, obj, scan_vert(m, obj));
    } else if (probe(m, obj, 0x20u, 0u)) {
        ease_x(m, obj, 0x00010000u);
    } else if (probe(m, obj, 0x28u, 0u)) {
        ease_x(m, obj, 0x00020000u);
    } else {
        uint32_t d0 = 0x00030000u;
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t <= 0) {
            if (sub_0CC24A(m, obj) == 0u && sub_0CC2BC(m, 0x000Au) == 0u) {
                uint32_t r = Rng_NextMem(m) & 0x1F00u;
                r = (r & 0xFFFF0000u) | (uint16_t)(r - 0x1000u);
                if ((rd8(m, obj + 0x1Cu) & 0x01u) == 0u)
                    r = ((r & 0xFFFF0000u) | (uint16_t)(r + 0x8000u)) | 0x0D1D0000u;
                else
                    r |= 0x121D0000u;
                Obj_Spawn(m, obj, 0x000D901Au, 9u, 2u, r);
            }
            uint32_t a1 = rd32(m, obj + 0x50u);
            int16_t dx = (int16_t)(uint16_t)(rd16(m, a1 + 0x14u) - rd16(m, obj + 0x14u));
            if (dx <= 0x34 && dx >= -0x44) {
                sub_0D7D48(m, obj);
                d0 = 0u;
            }
        }
        ease_x(m, obj, d0);
        ease_y(m, obj, 0u);
    }

    if ((rd8(m, obj + 0x1Cu) & 0x01u) == 0u) {
        if ((int16_t)rd16(m, obj + 0x14u) <= -0x20) { Obj_Free(m, obj); return 0; }
    } else {
        if ((int16_t)rd16(m, obj + 0x14u) >= 0x140) { Obj_Free(m, obj); return 0; }
    }
    wr32(m, obj + 0x00u, 0x000D7A86u);
    return 0;

die:
    play_sound(m, (uint16_t)rd8(m, obj + 0x65u));
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, sub_13E4D0(m, obj, 2u));
    wr32(m, obj + 0x00u, 0x000D7C66u);
    return 0;
}
