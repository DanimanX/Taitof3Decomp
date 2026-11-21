#include "obj_CAFB8.h"
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

/* ROM 0x0CB14A: centre on screen, scaled by zoom; returns D1's high word
 * as left by Obj_DrawSprite (multi-sprite defs: cols-1; single: unchanged). */
static uint16_t sub_0CB14A(const f3mem_t *m, uint32_t obj, uint16_t d1_hi)
{
    wr16(m, obj + 0x14u, 0x00A0u);
    wr16(m, obj + 0x18u, 0x0074u);
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0 = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t prod = (int32_t)(int16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (uint16_t)ASR16((uint16_t)(uint32_t)prod, 5u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
    d0 = (uint16_t)(0u - (uint16_t)(d0 << 2));
    wr16(m, F3_A5 + 0x1D5Cu, d0);
    uint32_t def = rd32(m, obj + 0x10u);
    if (rd32(m, def) != 0x00010001u)
        d1_hi = (uint16_t)(rd16(m, def) - 1u); /* bfins d0,d1{0:16} in 0x1282 */
    Obj_DrawSprite(m, obj);
    return d1_hi;
}

/* ROM 0x0CB180: every other frame spawn four 0xCB2C6 at 90-degree steps */
static void sub_0CB180(const f3mem_t *m, uint32_t obj, uint16_t d1_hi)
{
    if (rd16(m, F3_A5 + 0x0878u) & 2u)
        return;
    uint16_t d1w = (uint16_t)(rd16(m, F3_A5 + 0x0878u) << 9);
    for (int i = 0; i < 4; i++) {
        Obj_Spawn(m, obj, 0x000CB2C6u, 1u, 2u, ((uint32_t)d1_hi << 16) | d1w);
        d1w = (uint16_t)(d1w + 0x4000u);
    }
}

/* ROM 0x0CAFB8 */
int Obj_CAFB8_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, 0x000CB0F8u);
    /* D1 high word on entry from the object runner is 0 in all captures */
    uint16_t d1_hi = sub_0CB14A(m, obj, 0u);
    sub_0CB180(m, obj, d1_hi);
    uint16_t z = (uint16_t)(rd16(m, obj + 0x1Eu) + 0x0404u);
    wr16(m, obj + 0x1Eu, z);
    if (z <= 0xF8F8u) {
        wr32(m, obj + 0x00u, 0x000CAFB8u);
        return 0;
    }
    play_sound(m, 0x0019u);
    wr8(m, F3_A5 + 0x1D58u, (uint8_t)(rd8(m, F3_A5 + 0x1D58u) & (uint8_t)~0x02u));
    wr32(m, obj + 0x10u, 0x00028740u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x14u, 0u);
    wr16(m, obj + 0x18u, 0u);
    wr8(m, obj + 0x22u, 6u);
    wr16(m, F3_A5 + 0x1D5Cu, 0x00A0u);
    wr16(m, obj + 0x2Cu, 0x0064u);
    wr16(m, obj + 0x30u, 0x0038u);
    wr16(m, obj + 0x2Eu, 0x00DBu);
    wr16(m, obj + 0x32u, 0x00AFu);
    wr32(m, obj + 0x28u, 0x00006000u);
    wr8(m, F3_A5 + 0x1D58u, (uint8_t)(rd8(m, F3_A5 + 0x1D58u) | 0x04u));
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x00u, 0x000CB054u);
    return 0;
}
