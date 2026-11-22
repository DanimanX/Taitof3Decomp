#include "obj_CDF84.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* D0 at tick entry: the 0x0104B8/0x010532 dispatcher's group counter */
static uint32_t entry_d0(uint32_t obj)
{
    uint32_t g = (obj - OBJ_BASE) / OBJ_GROUP_SIZE;
    return g >= 11u ? 0u : 10u - g;
}

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

/* ROM 0x0CD3D6 */
static void next_record(const f3mem_t *m, uint32_t obj, uint16_t d0)
{
    uint32_t a1 = rd32(m, 0x000CD444u + (uint32_t)(int32_t)(int16_t)d0 * 4u);
    uint16_t snd = rd16(m, a1); a1 += 2u;
    if (snd != 0u)
        play_sound(m, snd);
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x00u, 0x000CD412u);
}

/* ROM 0x0CE16E: pack the position history entry */
static void sub_0CE16E(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    d0 = (d0 & 0x3u)
       | ((uint32_t)(rd16(m, obj + 0x14u) & 0x3FFu) << 22)
       | ((uint32_t)(rd16(m, obj + 0x18u) & 0x1FFu) << 13)
       | ((uint32_t)(rd16(m, obj + 0x20u) & 0xFFu) << 5)
       | ((uint32_t)(rd16(m, obj + 0x28u) & 0x1u) << 4)
       | ((uint32_t)(rd8(m, obj + 0x3Fu) & 0x3u) << 2);
    int16_t i = (int16_t)rd16(m, obj + 0x3Au);
    wr32(m, obj + 0x4Cu + (uint32_t)(int32_t)i * 4u, d0);
    wr16(m, obj + 0x3Au, (uint16_t)((rd16(m, obj + 0x3Au) + 1u) & 3u));
}

/* ROM 0x0CDF84 */
int Obj_CDF84_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CE16E(m, obj, entry_d0(obj));
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj, 0xCDF84u);
        return 0;
    }
    uint32_t d0 = rd32(m, obj + 0x48u);
    if (d0 != 0u)
        wr32(m, d0 + 0x44u, 0u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr16(m, obj + 0x42u, 0x0Au);
    if ((rd8(m, obj + 0x3Fu) & 0x01u) != 0u) {
        next_record(m, obj, 1u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
