#include "obj_E6082.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CEDE4: initialize object combat/hit fields from descriptor table */
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
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
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

/* ROM 0x0D2D44: (D0, D1) = per-mode speed tables scaled by D1w (0..50) */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
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

/* ROM 0x0E6082 */
int Obj_E6082_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t speed = rd16(m, obj + 0x60u);
    if (speed > 50u)
        return -1;

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0018u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u));
    sub_0CEDE4(m, obj, 0x000EABAAu);
    sub_0D0678(m, obj, 0x000E612Au);

    uint32_t d0, d1;
    sub_0D2D44(m, rd16(m, obj + 0x58u), speed, &d0, &d1);
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    Obj_DrawSprite(m, obj);

    int16_t cmd = sub_13F238(m, 0x001B);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }

    wr32(m, obj + 0x00u, 0x000E60EAu);
    return 0;
}
