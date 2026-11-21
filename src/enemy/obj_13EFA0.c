#include "obj_13EFA0.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap */
static uint16_t sub_13F238(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return (uint16_t)cmd;
}

/* ROM 0x003CE0: channel volume command 0x89 for sound cmd's channel */
static void sub_003CE0(const f3mem_t *m, uint16_t cmd, uint8_t b2, uint8_t b3, uint8_t b4)
{
    uint32_t a0 = rd32(m, rd32(m, F3_A5 - 0x1448u) + (uint32_t)((int32_t)(int16_t)cmd * 4)) + 1u;
    uint16_t d0 = (uint16_t)(rd8(m, a0) & 0x7Fu);
    if (rd8(m, F3_A5 - 0x1442u + d0) == 0u)
        return;
    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1);
    if (b3 & 0x80u)
        b3 = rd8(m, a0 + 1u);
    if (b4 & 0x80u)
        b4 = rd8(m, a0 + 1u);
    uint32_t a2 = F3_A5 - 0x13C2u;
    wr16(m, a2, 0x0789u);
    wr8(m, a2 + 2u, rd8(m, a0));
    wr8(m, a2 + 3u, b3);
    wr8(m, a2 + 4u, b4);
    wr8(m, a2 + 5u, b2);
    wr8(m, a2 + 6u, 0u);
    Sound_WriteScratch(m, &d1);
    wr16(m, F3_DPRAM_BASE + 0x480u, (uint16_t)(d1 + d1));
}

enum { SND_PLAY, SND_FORCED, SND_VOL };

/* Runs a sound call with a temporary DPRAM when the harness has none. */
static void snd(const f3mem_t *m, int kind, uint16_t cmd, uint8_t b2, uint8_t b3, uint8_t b4)
{
    uint8_t tmp_dpram[F3_DPRAM_SIZE];
    f3mem_t tmp_m;
    const f3mem_t *mm = m;
    if (!m->dpram) {
        for (unsigned i = 0; i < F3_DPRAM_SIZE; i++)
            tmp_dpram[i] = 0u;
        tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        mm = &tmp_m;
    }
    if (kind == SND_PLAY)
        Sound_PlayCmd(mm, (int16_t)cmd);
    else if (kind == SND_FORCED)
        Sound_PlayCmdForced(mm, (int16_t)cmd);
    else
        sub_003CE0(mm, cmd, b2, b3, b4);
}

/* ROM 0x13EFA0 (group 11): stage music cue sequencer */
int Obj_13EFA0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t d0 = rd16(m, obj + 0x06u);

    if (d0 == rd16(m, a1 + 2u)) {
        uint16_t d1 = rd16(m, a1);
        if ((int16_t)d1 >= 0) {
            wr16(m, F3_A5 + 0x3302u, d1);
            snd(m, SND_PLAY, sub_13F238(m, d1), 0, 0, 0);
            if (rd16(m, F3_A5 + 0x32DEu) == 7u)
                snd(m, SND_VOL, sub_13F238(m, d1), 0x45u, 0x00u, 0xFFu);
        }
    }

    /* 0x13EFF4 */
    if (d0 == (uint16_t)(rd16(m, obj + 0x22u) + rd16(m, a1 + 4u))) {
        uint16_t d2 = rd16(m, a1 + 6u);
        if ((int16_t)d2 >= 0 && d2 != rd16(m, F3_A5 + 0x3302u)) {
            uint16_t d1 = rd16(m, F3_A5 + 0x3302u);
            if ((int16_t)d1 > 0)
                snd(m, SND_VOL, sub_13F238(m, d1), 0x42u, 0xFFu, 0x00u);
        }
    }

    /* 0x13F038 */
    if (d0 == (uint16_t)(rd16(m, obj + 0x22u) + rd16(m, a1 + 4u) + 0x12Cu)) {
        uint16_t d2 = rd16(m, a1 + 6u);
        if ((int16_t)d2 >= 0 && d2 != rd16(m, F3_A5 + 0x3302u)) {
            uint16_t d1 = rd16(m, F3_A5 + 0x3302u);
            if ((int16_t)d1 > 0)
                snd(m, SND_FORCED, sub_13F238(m, d1), 0, 0, 0);
        }
    }

    /* 0x13F074 */
    if (d0 == (uint16_t)(rd16(m, obj + 0x22u) + rd16(m, a1 + 8u))) {
        uint16_t d1 = rd16(m, a1 + 6u);
        wr16(m, F3_A5 + 0x3304u, d1);
        if ((int16_t)d1 >= 0 && d1 != rd16(m, F3_A5 + 0x3302u))
            snd(m, SND_PLAY, sub_13F238(m, d1), 0, 0, 0);
    }

    /* 0x13F0A8 */
    if (rd8(m, F3_A5 - 0x144Bu) & 0x10u) {
        uint16_t d1 = rd16(m, F3_A5 + 0x3304u);
        if ((int16_t)d1 >= 0) {
            wr16(m, F3_A5 + 0x3304u, 0xFFFFu);
            snd(m, SND_FORCED, sub_13F238(m, d1), 0, 0, 0);
        }
    }

    /* 0x13F0D2 */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 1u));
    if (rd8(m, F3_A5 - 0x144Bu) & 0x04u) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x0013EFA0u);
    return 0;
}
