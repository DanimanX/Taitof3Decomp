#include "obj_10E316.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap hook */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x10E316 */
int Obj_10E316_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x10E316: btst #5,(A5-0x144B); bne -> 0x0CEE74 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x10E322 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    uint32_t y = rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x14u, y);
    if ((int32_t)y < 0) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x10E338 */
    if (rd8(m, obj + 0x06u) == 0u) {
        /* 0x10E340 */
        uint8_t t0 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, t0);
        if (t0 != 0u)
            return 0;
        /* 0x10E348 */
        wr8(m, obj + 0x23u, 0x3Cu);
        wr8(m, obj + 0x21u, 0x01u);
        wr8(m, obj + 0x06u, 0x01u);
        /* 0x10E35A: move.w #0x27,-(SP); jsr 0x13F238; jsr 0x39D8 */
        int16_t cmd = sub_13F238(m, 0x0027);
        if (m->dpram) {
            Sound_PlayCmd(m, cmd);
        } else {
            uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
            f3mem_t tmp_m = *m;
            tmp_m.dpram = tmp_dpram;
            Sound_PlayCmd(&tmp_m, cmd);
        }
        return 0;
    }

    /* 0x10E370 */
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t == 0u) {
        /* 0x10E3BE */
        wr8(m, obj + 0x23u, 0x3Cu);
        wr8(m, obj + 0x06u, 0u);
        return 0;
    }

    /* 0x10E378 */
    uint8_t k = (uint8_t)(rd8(m, obj + 0x21u) - 1u);
    wr8(m, obj + 0x21u, k);
    if (k != 0u)
        return 0;
    wr8(m, obj + 0x21u, 0x07u);

    /* 0x10E386: pea (0x0).w; move.w #2,-(SP); move.w #9,-(SP);
     * move.l #0xD936A,-(SP); jsr 0x1057C */
    uint32_t spawned = Obj_Spawn(m, obj, 0x000D936Au, 9u, 2u, 0u);
    if ((int32_t)spawned < 0)
        return 0;
    /* 0x10E3A8 */
    wr32(m, spawned + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, spawned + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, spawned + 0x58u, rd16(m, obj + 0x58u));
    return 0;
}
