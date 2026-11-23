#include "obj_DC6D0.h"
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

/* ROM 0x0CEDE4 */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;

    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));   /* move.l (A1),(0x30,A0): A1 not advanced */
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0E3A */

/* ROM 0x0D2D44: (D0, D1) = per-mode speed tables scaled by D2w */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    /* jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) */
    uint32_t d0 = rd32(m, 0x000D2D64u + (uint32_t)((int32_t)(int16_t)d0w * 4));
    uint32_t d1 = rd32(m, 0x000D2ECCu + (uint32_t)((int32_t)(int16_t)d0w * 4));
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x0D4A12 */
static int sub_0D4A12(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t *out_d0)
{
    uint32_t d0 = rd32(m, 0x000D4A5Au + (uint32_t)((int32_t)(int16_t)d1w * 4));
    if (d0w > 50u) {
        /* unimplemented: ROM 0x0D4E5A */
        return -1;
    }
    /* Jump table 0x0D4E5A [0..50] -> 0x0D4FF4..0x0D5396: exact D0 * k mod 2^32 */
    d0 = (uint32_t)(d0 * (uint32_t)d0w);
    *out_d0 = (uint16_t)(d0 >> 16);
    return 0;
}

/* ROM 0x0DC6D0 -> 0x0DC5E2 */
int Obj_DC6D0_Tick(const f3mem_t *m, uint32_t obj)
{
    play_sound(m, 0x0033u);

    /* 0x0DC5E2 */
    if (rd16(m, obj + 0x58u) > 0x0167u || rd16(m, obj + 0x60u) > 0x0032u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    sub_0CEDE4(m, obj, 0x000D8FE2u);
    wr16(m, obj + 0x1Eu, rd16(m, obj + 0x10u));
    uint16_t d0w = 0;
    if (sub_0D4A12(m, 0x0018u, (uint16_t)rd8(m, obj + 0x1Eu), &d0w) < 0)
        return -1;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));

    uint32_t a1 = 0x000DC124u;
    uint16_t r = Obj_Next4Way(m);
    if (r > 0x0033u) {
        a1 = 0x000DC1CCu;
        if (r > 0x0099u)
            a1 = 0x000DC238u;
    }
    sub_0D0678(m, obj, a1);

    uint32_t d0, d1;
    sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &d0, &d1);
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000DC674u);
    return 0;
}
