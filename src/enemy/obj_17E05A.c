#include "obj_17E05A.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 + 0x39D8: sound command (+0x53 in the alternate sound mode) */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x184888: initialise the object's stats from the record at A1 */
static void sub_184888(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr8(m, obj + 0x34u, 0x08u);
    wr8(m, obj + 0x35u, 0x00u);
    wr16(m, obj + 0x64u, 0u);
    wr16(m, obj + 0x1Cu, rd16(m, a1 + 0x00u));
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x02u));
    wr16(m, obj + 0x24u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x28u, rd16(m, a1 + 0x06u));
    wr32(m, obj + 0x2Cu, rd32(m, a1 + 0x08u));
    wr32(m, obj + 0x30u, rd32(m, a1 + 0x0Cu));
    wr16(m, obj + 0x62u, rd16(m, a1 + 0x10u));
    wr32(m, obj + 0x66u, rd32(m, a1 + 0x12u));
    wr8(m, obj + 0x22u, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) / 10u);   /* divul.l #10 */
}

/* Animation state machines, table 0x179A5C (init, tick) per index (+0x40).
 * Only the indices these objects use are translated; others return -1. */

/* ROM 0x179550 (index 0 init) */
static void anim0_init(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
}

/* ROM 0x1798D4 (index 10 tick): pick one of 32 direction frames
 * {frame.l, colour.w} from +0x3A by the heading byte +0x20 */
static void anim10_tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
    uint32_t a1 = rd32(m, obj + 0x3Au) + (uint32_t)(d0 * 6u);
    wr32(m, obj + 0x10u, rd32(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
}

/* ROM 0x179614 (index 10 init) */
static void anim10_init(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);
    anim10_tick(m, obj);
}

static int anim_init(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    switch (idx) {
    case 0u:  anim0_init(m, obj); return 0;
    case 10u: anim10_init(m, obj); return 0;
    default:  return -1;
    }
}

/* Load script at a1 ({index.w, data...}) and run its init (jsr via table). */
static int anim_start(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1 + 2u);
    return anim_init(m, obj, d0);
}

/* ROM 0x17E060: start: sound 0x11, stats 0x17E112, anim 0x17E128,
 * movement script 0x17E160 */
static int lab_17E060(const f3mem_t *m, uint32_t obj)
{
    play_sound(m, 0x11);
    sub_184888(m, obj, 0x0017E112u);
    if (anim_start(m, obj, 0x0017E128u) < 0)
        return -1;
    wr32(m, obj + 0x44u, 0x0017E160u);
    wr16(m, obj + 0x48u, rd16(m, 0x0017E164u));
    wr16(m, obj + 0x4Au, 0u);
    wr32(m, obj + 0x00u, 0x0017C05Eu);
    return 0;
}

/* ROM 0x17E05A: wait +6 frames, then start */
int Obj_17E05A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x0017E05Au);
        return 0;
    }
    return lab_17E060(m, obj);
}
