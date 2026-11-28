#include "sound_cmd.h"
#include <stdint.h>
#include <stdlib.h>

static void snd_play_type0_3a2c(const f3mem_t *m, uint32_t a0, uint16_t *idx)
{
    /* ROM 0x3A2C */
    uint8_t ch = rd8(m, a0);
    uint32_t scratch = F3_A5 - 0x13C2u;

    if ((int8_t)ch >= 0) {
        wr8(m, (F3_A5 - 0x1442u) + (uint32_t)ch, ch);

        wr16(m, scratch, 0x0381u);
        wr8(m, scratch + 2u, rd8(m, a0));
        Sound_WriteScratch(m, idx);

        wr16(m, scratch, 0x0486u);
        wr8(m, scratch + 2u, rd8(m, a0));
        wr8(m, scratch + 3u, rd8(m, a0 + 1u));
        Sound_WriteScratch(m, idx);
    } else {
        /* ROM 0x3A5C */
        uint8_t d0 = (uint8_t)(ch & 0x7Fu);
        uint8_t d2 = d0;

        wr8(m, (F3_A5 - 0x1442u) + (uint32_t)d0, rd8(m, a0));

        wr16(m, scratch, 0x0381u);
        wr8(m, scratch + 2u, rd8(m, a0));
        Sound_WriteScratch(m, idx);

        wr16(m, scratch, 0x0486u);
        wr8(m, scratch + 2u, rd8(m, a0));
        wr8(m, scratch + 3u, rd8(m, a0 + 1u));
        Sound_WriteScratch(m, idx);

        wr16(m, scratch, 0x0486u);
        wr8(m, scratch + 2u, d2);
        wr8(m, scratch + 3u, rd8(m, a0 + 1u));
        Sound_WriteScratch(m, idx);
    }
}

static void snd_play_type1_3a9e(const f3mem_t *m, uint32_t a0, uint16_t *idx)
{
    /* ROM 0x3A9E */
    uint16_t d0 = (uint16_t)(rd8(m, a0) & 0x7Fu);
    uint32_t scratch = F3_A5 - 0x13C2u;

    wr8(m, (F3_A5 - 0x1442u) + (uint32_t)d0, rd8(m, a0));

    wr16(m, scratch, 0x0381u);
    wr8(m, scratch + 2u, rd8(m, a0));
    Sound_WriteScratch(m, idx);

    wr16(m, scratch, 0x0486u);
    wr8(m, scratch + 2u, rd8(m, a0));
    wr8(m, scratch + 3u, rd8(m, a0 + 1u));
    Sound_WriteScratch(m, idx);

    wr16(m, scratch, 0x0488u);
    wr8(m, scratch + 2u, rd8(m, a0));
    wr8(m, scratch + 3u, rd8(m, a0 + 2u));
    Sound_WriteScratch(m, idx);
}

static void snd_play_type2_3ae2(const f3mem_t *m, uint32_t a0, uint16_t *idx)
{
    /* ROM 0x3AE2 */
    uint16_t d0 = rd8(m, a0);
    uint32_t scratch = F3_A5 - 0x13C2u;
    uint32_t slot;
    uint8_t prev;

    d0 = (uint16_t)(d0 - 1u);
    d0 = (uint16_t)((uint32_t)d0 << 3);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + rd8(m, a0 + 2u)));

    slot = F3_A5 + 0x0817u + (uint32_t)(int32_t)(int16_t)d0;
    prev = rd8(m, slot);
    if (prev != 0u) {
        wr16(m, scratch, 0x058Fu);
        wr8(m, scratch + 2u, rd8(m, a0));
        wr8(m, scratch + 3u, rd8(m, a0 + 2u));
        wr8(m, scratch + 4u, prev);
        Sound_WriteScratch(m, idx);
    }

    wr16(m, scratch, 0x068Du);
    wr8(m, scratch + 2u, rd8(m, a0));
    wr8(m, scratch + 3u, rd8(m, a0 + 2u));
    wr8(m, scratch + 4u, rd8(m, a0 + 3u));
    wr8(m, scratch + 5u, rd8(m, a0 + 4u));
    Sound_WriteScratch(m, idx);

    wr16(m, scratch, 0x068Eu);
    wr8(m, scratch + 2u, rd8(m, a0));
    wr8(m, scratch + 3u, rd8(m, a0 + 2u));
    wr8(m, scratch + 4u, rd8(m, a0 + 5u));
    wr8(m, scratch + 5u, rd8(m, a0 + 1u));
    Sound_WriteScratch(m, idx);

    wr8(m, slot, rd8(m, a0 + 5u));
}

/* ROM 0x3B4E: copy the counted byte string at A0 into `base` at ring index
 * *idx (0x003B54 move.b (a0)+,(a1,d1.w)). A1 is the caller's destination base.
 * Returns the number of bytes consumed from A0 (its advance at the rts). */
uint16_t Sound_CopyToRing(const f3mem_t *m, uint32_t a0, uint32_t base, uint16_t *idx)
{
    uint16_t d0 = rd8(m, a0++);
    uint16_t d1 = *idx;
    uint16_t used = 1u;

    for (;;) {
        d0 = (uint16_t)(d0 - 1u);
        if (d0 == 0xFFFFu) {
            break;
        }
        wr8(m, base + (uint32_t)(int32_t)(int16_t)d1, rd8(m, a0++));
        d1 = (uint16_t)((d1 + 1u) & 0x3FFu);
        used++;
    }
    *idx = d1;
    return used;
}

static void snd_copy_3b4e(const f3mem_t *m, uint32_t a0, uint16_t *idx)
{
    (void)Sound_CopyToRing(m, a0, F3_DPRAM_BASE, idx);
}

static void snd_forced_type01_3ba8(const f3mem_t *m, uint32_t a0, uint16_t *idx)
{
    /* ROM 0x3BA8 */
    uint8_t d0 = (uint8_t)(rd8(m, a0) & 0x7Fu);
    uint32_t slot = (F3_A5 - 0x1442u) + (uint32_t)d0;
    uint8_t d2 = rd8(m, slot);
    uint32_t scratch = F3_A5 - 0x13C2u;

    if (d2 == 0u) {
        return;
    }

    wr8(m, slot, 0u);

    wr16(m, scratch, 0x0486u);
    wr8(m, scratch + 2u, d2);
    wr8(m, scratch + 3u, 0u);
    Sound_WriteScratch(m, idx);

    wr16(m, scratch, 0x0382u);
    wr8(m, scratch + 2u, d2);
    Sound_WriteScratch(m, idx);
}

static void snd_forced_type2_3be0(const f3mem_t *m, uint32_t a0, uint16_t *idx)
{
    /* ROM 0x3BE0 */
    uint16_t d0 = rd8(m, a0);
    uint32_t slot;
    uint8_t prev;
    uint32_t scratch = F3_A5 - 0x13C2u;

    d0 = (uint16_t)(d0 - 1u);
    d0 = (uint16_t)((uint32_t)d0 << 3);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + rd8(m, a0 + 2u)));

    slot = F3_A5 + 0x0817u + (uint32_t)(int32_t)(int16_t)d0;
    prev = rd8(m, slot);
    if (prev == 0u) {
        return;
    }

    wr8(m, slot, 0u);

    wr16(m, scratch, 0x058Fu);
    wr8(m, scratch + 2u, rd8(m, a0));
    wr8(m, scratch + 3u, rd8(m, a0 + 2u));
    wr8(m, scratch + 4u, prev);
    Sound_WriteScratch(m, idx);
}

static void snd_stop_part2_3c6e(const f3mem_t *m, uint16_t *idx)
{
    /* ROM 0x3C6E */
    uint16_t d2 = 1u;
    uint16_t d4 = 4u;

    for (;;) {
        uint16_t d3;

        d4 = (uint16_t)(d4 - 1u);
        if (d4 == 0xFFFFu) {
            break;
        }

        d3 = 1u;
        do {
            uint16_t d0 = d2;
            uint8_t val;

            d0 = (uint16_t)(d0 - 1u);
            d0 = (uint16_t)((uint32_t)d0 << 3);
            d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + (uint8_t)d3));

            val = rd8(m, F3_A5 + 0x0817u + (uint32_t)(int32_t)(int16_t)d0);
            if (val != 0u) {
                uint32_t scratch = F3_A5 - 0x13C2u;

                wr16(m, scratch, 0x058Fu);
                wr8(m, scratch + 2u, (uint8_t)d2);
                wr8(m, scratch + 3u, (uint8_t)d3);
                wr8(m, scratch + 4u, val);
                Sound_WriteScratch(m, idx);
            }

            d3 = (uint16_t)(d3 + 1u);
        } while (d3 <= 8u);

        d2 = (uint16_t)(d2 + 1u);
    }
}

void Sound_ResetDpram(const f3mem_t *m)
{
    /* ROM 0x3960 */
    uint32_t a0 = F3_DPRAM_BASE;
    uint16_t d0 = 0x01FFu;

    /* Hardware sound-reset assert write to 0x00C80100 (move.w #0, (0xC80100).l) ignored. */

    do {
        wr32(m, a0, 0u);
        a0 += 4u;
        d0 = (uint16_t)(d0 - 1u);
    } while (d0 != 0xFFFFu);

    wr32(m, F3_A5 - 0x1448u, 0x000064E8u);
    wr8(m, F3_A5 - 0x1444u, 0u);

    /* Hardware sound-reset release write to 0x00C80000 (move.w #0, (0xC80000).l) ignored. */
}

/* ROM 0x3998: copy the counted byte string at A5-0x13C2 into `base` at the
 * ring index *idx (0x0039A0 move.b d0,(a1,d1.w)). The ROM's destination base is
 * the A1 register, not a fixed 0xC00000: the routine is shared by the DPRAM
 * callers (base = F3_DPRAM_BASE) and by captures whose injected call leaves
 * another base there. Returns the number of bytes consumed from A5-0x13C2 (the
 * A2 advance the ROM leaves behind). */
uint16_t Sound_WriteScratchBase(const f3mem_t *m, uint32_t base, uint16_t *idx)
{
    uint32_t a2 = F3_A5 - 0x13C2u;
    uint16_t d1 = *idx;
    uint16_t d0 = rd8(m, a2++);
    uint16_t used = 1u;

    wr8(m, base + (uint32_t)(int32_t)(int16_t)d1, (uint8_t)d0);
    d0 = (uint16_t)(d0 - 1u);
    for (;;) {
        d1 = (uint16_t)((d1 + 1u) & 0x3FFu);
        d0 = (uint16_t)(d0 - 1u);
        if (d0 == 0xFFFFu) {
            break;
        }
        wr8(m, base + (uint32_t)(int32_t)(int16_t)d1, rd8(m, a2++));
        used++;
    }
    *idx = d1;
    return used;
}

void Sound_WriteScratch(const f3mem_t *m, uint16_t *idx)
{
    (void)Sound_WriteScratchBase(m, F3_DPRAM_BASE, idx);
}

void Sound_PlayCmd(const f3mem_t *m, int16_t cmd)
{
    /* ROM 0x39D8 */
    if (!m->dpram) {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
        return;
    }
    uint16_t d1;
    uint32_t a0;
    uint8_t type;

    if (rd8(m, F3_A5 - 0x1444u) != 0u) {
        return;
    }
    if (cmd < 0) {
        return;
    }
    if (cmd > (int16_t)rd16(m, 0x00006750u)) {
        return;
    }

    d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1);
    a0 = rd32(m, F3_A5 - 0x1448u);
    a0 = rd32(m, a0 + (uint32_t)((int32_t)cmd * 4));
    type = rd8(m, a0++);

    /* Jump table at ROM 0x3A22 */
    switch (type) {
    case 0u:
        snd_play_type0_3a2c(m, a0, &d1);
        break;
    case 1u:
        snd_play_type1_3a9e(m, a0, &d1);
        break;
    case 2u:
        snd_play_type2_3ae2(m, a0, &d1);
        break;
    case 3u:
        snd_copy_3b4e(m, a0, &d1);
        break;
    case 4u:
        /* ROM 0x3B64 */
        break;
    default:
        abort();
    }

    d1 = (uint16_t)(d1 + d1);
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);
}

void Sound_PlayCmdForced(const f3mem_t *m, int16_t cmd)
{
    /* ROM 0x3B66 */
    if (!m->dpram) {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmdForced(&tmp_m, cmd);
        return;
    }
    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1);
    uint32_t a0 = rd32(m, F3_A5 - 0x1448u);
    uint8_t type;

    a0 = rd32(m, a0 + (uint32_t)((int32_t)cmd * 4));
    type = rd8(m, a0++);

    /* Jump table at ROM 0x3B9E */
    switch (type) {
    case 0u:
    case 1u:
        snd_forced_type01_3ba8(m, a0, &d1);
        break;
    case 2u:
        snd_forced_type2_3be0(m, a0, &d1);
        break;
    case 3u:
    case 4u:
        /* ROM 0x3C10 */
        break;
    default:
        abort();
    }

    d1 = (uint16_t)(d1 + d1);
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);
}

void Sound_StopAllPart2(const f3mem_t *m)
{
    /* ROM 0x3C12 */
    if (!m->dpram) {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_StopAllPart2(&tmp_m);
        return;
    }
    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1);

    snd_stop_part2_3c6e(m, &d1);

    d1 = (uint16_t)(d1 + d1);
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);
}

void Sound_StopAll(const f3mem_t *m)
{
    /* ROM 0x3C24 */
    if (!m->dpram) {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_StopAll(&tmp_m);
        return;
    }
    uint16_t d1 = (uint16_t)(rd16(m, F3_DPRAM_BASE + 0x480u) >> 1);
    uint16_t d3 = 0u;
    uint16_t d2 = 0x007Fu;

    do {
        uint32_t slot = (F3_A5 - 0x1442u) + (uint32_t)(int32_t)(int16_t)d3;
        uint8_t d4 = rd8(m, slot);

        if (d4 != 0u) {
            uint32_t scratch = F3_A5 - 0x13C2u;

            wr8(m, slot, 0u);

            wr16(m, scratch, 0x0486u);
            wr8(m, scratch + 2u, d4);
            wr8(m, scratch + 3u, 0u);
            Sound_WriteScratch(m, &d1);

            wr16(m, scratch, 0x0382u);
            wr8(m, scratch + 2u, d4);
            Sound_WriteScratch(m, &d1);
        }

        d3 = (uint16_t)(d3 + 1u);
        d2 = (uint16_t)(d2 - 1u);
    } while (d2 != 0xFFFFu);

    snd_stop_part2_3c6e(m, &d1);

    d1 = (uint16_t)(d1 + d1);
    wr16(m, F3_DPRAM_BASE + 0x480u, d1);
}

void Sound_SetVolume(const f3mem_t *m, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    /* ROM 0x3CBA */
    wr8(m, 0x00C007F8u, a);
    wr8(m, 0x00C007F9u, b);
    wr8(m, 0x00C007FAu, c);
    wr8(m, 0x00C007FBu, d);
}
