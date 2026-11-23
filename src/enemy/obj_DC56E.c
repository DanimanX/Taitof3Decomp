#include "obj_DC56E.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../audio/sound_cmd.h"
#include "../stage/stage_timeline.h"

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

/* ROM 0x0D0E3A: 4-way interleaved global counter in A5+0x1D60 */

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

/* ROM 0x13F238: sound command remap */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x0DC49A: common spawn body (shared with the dying sequence) */
static void sub_0DC49A(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DC49A: spawn window checks (cmpi.w/bhi) */
    if (rd16(m, obj + 0x58u) > 0x0167u || rd16(m, obj + 0x60u) > 0x0032u) {
        Obj_StartDying(m, obj);
        return;
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0018u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0018u));
    sub_0CEDE4(m, obj, 0x000D8FE2u);

    /* 0x0DC4C8 */
    uint32_t a1 = (uint32_t)(0x0DC4CAu - 0x3A6u);
    uint16_t r = Obj_Next4Way(m);
    if (r > 0x0033u) {
        a1 = (uint32_t)(0x0DC4DCu - 0x310u);
        if (r > 0x0099u)
            a1 = (uint32_t)(0x0DC4E8u - 0x2B0u);
    }

    /* 0x0DC4EA */
    sub_0D0678(m, obj, a1);

    /* 0x0DC4F0: angle = obj+0x58 + (rnd & 0x3F) - 0x20, wrapped to [0,0x168) */
    int16_t ang = (int16_t)(uint16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20u + rd16(m, obj + 0x58u));
    if (ang < 0)
        ang = (int16_t)(ang + 0x168);
    else if (ang >= 0x168)
        ang = (int16_t)(ang - 0x168);

    /* 0x0DC51A */
    uint32_t d0, d1;
    sub_0D2D44(m, (uint16_t)ang, rd16(m, obj + 0x60u), &d0, &d1);
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);

    /* 0x0DC52C */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000DC542u);
}

/* ROM 0x0DC56E */
int Obj_DC56E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0DC56E: move.w #0x32,-(SP); jsr 0x13F238; jsr Sound_PlayCmd; addq sp */
    int16_t cmd = sub_13F238(m, 0x0032);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    /* 0x0DC59A */
    sub_0DC49A(m, obj);
    return 0;
}
