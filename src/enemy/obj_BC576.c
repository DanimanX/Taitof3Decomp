#include "obj_BC576.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x0BC576 */
int Obj_BC576_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC576 */
    uint16_t d0w = rd16(m, obj + 0x60u);
    if ((int16_t)d0w > 0x40)
        goto lab_bc5f2;
    if (d0w != 0x40)
        goto lab_bc59a;

    /* 0x0BC586: SFX 0x31 */
    if (m->dpram) {
        Sound_PlayCmd(m, sub_13F238(m, 0x0031));
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, sub_13F238(m, 0x0031));
    }

lab_bc59a:
    /* 0x0BC59A */
    d0w = (uint16_t)(d0w << 1);
    wr8(m, obj + 0x1Fu, (uint8_t)d0w);
    wr8(m, obj + 0x1Eu, (uint8_t)d0w);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));

    uint32_t d0 = (uint32_t)((uint32_t)rd8(m, obj + 0x1Fu) * 0x80u);
    d0 = d0 << 8;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
    d0 = (uint32_t)((uint32_t)rd8(m, obj + 0x1Fu) * 0x40u);
    d0 = d0 << 8;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);

    /* 0x0BC5D4 */
    d0w = rd16(m, obj + 0x60u);
    if (d0w & 0x0001u)
        goto lab_bc5f2;
    Obj_Animate(m, obj, 0x000BC61Cu);       /* lea (0x3a,PC) at 0x0BC5E0 */
    Obj_DrawSprite(m, obj);                 /* 0x0BC5EC */

lab_bc5f2:
    /* 0x0BC5F2 */
    d0w = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, d0w);
    if ((int16_t)d0w >= 0) {
        wr32(m, obj, 0x000BC576u);          /* 0x0BC56E */
        return 0;
    }

    /* 0x0BC5FA: forced SFX 0x31 then free */
    if (m->dpram) {
        Sound_PlayCmdForced(m, sub_13F238(m, 0x0031));
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmdForced(&tmp_m, sub_13F238(m, 0x0031));
    }
    Obj_Free(m, obj);                       /* 0x0BC60E */
    return 0;
}
