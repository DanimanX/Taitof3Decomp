#include "obj_BC70A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x39D8 via the 0x13F238 remap, with a scratch DPRAM when absent */
static void play_cmd(const f3mem_t *m, int16_t cmd)
{
    cmd = sub_13F238(m, cmd);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* 0x0BC790..0x0BC96C: one spawn + sound per matching ring-frame counter */
typedef struct {
    uint16_t frame;
    uint16_t group;
    uint16_t ngroups;
    uint32_t fn;
    uint32_t arg;
} ring_burst_t;

static const ring_burst_t k_burst[8] = {
    { 0x70, 0, 1, 0x000BCE38u, 0x00B00000u },
    { 0x68, 7, 2, 0x000BCF08u, 0x00200000u },
    { 0x40, 0, 1, 0x000BCE8Eu, 0x00780000u },
    { 0x34, 7, 2, 0x000BCF08u, 0x00900000u },
    { 0x28, 0, 1, 0x000BCE38u, 0x006C0000u },
    { 0x10, 0, 1, 0x000BCE8Eu, 0x00500000u },
    { 0x0E, 0, 1, 0x000BCE38u, 0x00D00000u },
    { 0x04, 7, 2, 0x000BCF08u, 0x00580000u },
};

/* ROM 0x0BC70A */
int Obj_BC70A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC70A: per-frame velocity ramp from the ring table at obj+0x58 */
    uint32_t a1 = rd32(m, obj + 0x58u);
    int32_t d0 = (int32_t)(int16_t)rd16(m, a1 + 2);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + (uint32_t)(d0 << 8));
    d0 = (int32_t)(int16_t)rd16(m, a1 + 4);
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + (uint32_t)(d0 << 8));
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, a1 + 6)));

    /* 0x0BC72E */
    uint8_t b = rd8(m, obj + 0x50u);
    wr8(m, obj + 0x1Fu, b);
    wr8(m, obj + 0x1Eu, b);
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + 1u));
    if (rd16(m, obj + 0x54u) == rd16(m, a1)) {
        wr32(m, obj + 0x58u, a1 + 8u);
        wr16(m, obj + 0x54u, 0u);
    }

    /* 0x0BC750 */
    Obj_AnimFrameStep(m, obj, rd32(m, obj + 0x5Cu));

    /* 0x0BC75A */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    uint32_t d = (uint32_t)rd8(m, obj + 0x1Fu) * 0x50u;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (d << 8));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
    d = (uint32_t)rd8(m, obj + 0x1Eu) * 0x40u;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (d << 8));

    Obj_DrawSprite(m, obj);

    /* 0x0BC790..0x0BC96C */
    for (int i = 0; i < 8; i++) {
        if (rd16(m, obj + 0x60u) == k_burst[i].frame) {
            Obj_Spawn(m, obj, k_burst[i].fn, k_burst[i].group, k_burst[i].ngroups,
                      k_burst[i].arg);
            play_cmd(m, 0x22);
        }
    }

    /* 0x0BC970 */
    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BC70Au);
        return 0;
    }

    /* 0x0BC978: ring died, spill the shrapnel */
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x44u) + 0x28u));
    wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x48u) + 0x20u));
    Obj_Spawn(m, obj, 0x000BCD90u, 0, 1, rd32(m, obj + 0x60u));
    play_cmd(m, 0x06);

    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x00380000u);
    Obj_Spawn(m, obj, 0x000BCE38u, 0, 1, rd32(m, obj + 0x48u));
    play_cmd(m, 0x22);

    wr32(m, obj + 0x00u, 0x000BCA00u);
    return 0;
}
