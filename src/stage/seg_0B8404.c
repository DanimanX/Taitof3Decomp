/* ROM 0x0B8404, kernel slot 1: legal notice, attract stage, demo prompt,
 * and high scores. The apparent TRAP at 0x0B8794 is part of a notice string.
 * The sequencer ends at the RTS at 0x0B907E. */
#include "../kernel/seg.h"
#include "../audio/sound_cmd.h"
#include "../common/fn_leaf1.h"
#include "../enemy/obj_1194C2.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/palette.h"
#include "../video/sprite.h"

static int dbra(uint32_t *d)
{
    seg_setw(d, (uint16_t)(*d - 1u));
    return (uint16_t)*d != 0xFFFFu;
}

static uint32_t sleep_at(const f3mem_t *m, f3regs_t *r, uint32_t trap_pc, uint32_t frames)
{
    seg_push32(m, r, frames);
    return trap_pc;
}

/* 0x14E8 (bank 0) or 0x1504 (bank 1), with a zero bank index. The block
 * relay at 0x1520 writes to hardware palette RAM (outside f3mem_t).
 * Follow the ROM block headers to reproduce the registers at its RTS. */
static void palette_bank_registers(const f3mem_t *m, f3regs_t *r,
                                   uint32_t table, uint32_t source)
{
    r->d[0] = 0u;
    r->a[0] = table;
    r->a[1] = source;
    r->a[2] = rd32(m, table);
    r->d[3] = rd32(m, table + 4u);
    if ((int32_t)r->d[3] < 0)
        return;

    r->a[0] = 0x00440000u;
    do {
        uint32_t block = r->a[2];
        uint32_t count = (uint16_t)(rd16(m, block) - 1u) + 1u;
        r->d[0] = rd16(m, block + 2u) + 4u * count;
        seg_setw(&r->d[1], (uint16_t)(rd16(m, block + 4u) + 4u * count));
        seg_setw(&r->d[2], 0xFFFFu);
        r->a[2] += 6u;
    } while (dbra(&r->d[3]));
}

/* 0x5B0C leaves D2 alone after a full rotation across the eight pages.
 * D0.w is the last line control word, D1.w and D3 have run down to -1. */
static void init_line_ram(const f3mem_t *m, f3regs_t *r)
{
    Gfx_InitLineRam(m);
    uint32_t last = rd16(m, F3_A5 - 0x5D58u) ? 0x00624600u : 0x006247FEu;
    r->d[0] = 0x00180000u | rd16(m, last);
    seg_setw(&r->d[1], 0xFFFFu);
    r->d[3] = 0x0000FFFFu;
    r->a[0] = 0x00005C38u;
    r->a[1] = 0x00621000u;
}

/* TRAP #1 at 0x1E76, for the fixed nonzero slots requested here. The two
 * pushed arguments leave the caller's top long as the new task's parameter.
 * Construct the child's exception frame and saved registers on its stack. */
static void spawn_task(const f3mem_t *m, f3regs_t *r, uint32_t slot, uint32_t entry)
{
    uint32_t active_addr = r->a[5] - 0x1904u;
    uint32_t active = rd32(m, active_addr);
    uint32_t bit = 1u << slot;
    if (active & bit) {
        r->d[0] = 0u;
        return;
    }

    r->d[0] = slot;
    wr32(m, active_addr, active | bit);
    wr16(m, r->a[5] - 0x1878u + 2u * slot, 0xFFFFu);
    wr32(m, r->a[5] - 0x18F8u + 4u * slot,
         (uint32_t)rd16(m, r->a[5] - 0x1906u) << 16);

    uint32_t sp = rd32(m, 0x00002368u + 4u * slot) - 76u;
    for (unsigned i = 0; i < 8u; ++i)
        wr32(m, sp + 4u * i, r->d[i]);
    for (unsigned i = 0; i < 7u; ++i)
        wr32(m, sp + 32u + 4u * i, r->a[i]);
    wr16(m, sp + 60u, 0x2000u);
    wr32(m, sp + 62u, entry);
    wr16(m, sp + 66u, 0u);
    wr32(m, sp + 68u, 0x00001E48u);
    wr32(m, sp + 72u, rd32(m, r->a[7]));
    wr32(m, r->a[5] - 0x1948u + 4u * slot, sp);
}

/* 0x13D94C saves D0-D3/A0 and restores D7 after the line fill. Its last
 * score row determines the D4, D5 and A2 left by the route-letter loop. */
static void draw_high_scores(const f3mem_t *m, f3regs_t *r)
{
    Stage_DrawHighScores13D94C(m);
    uint32_t last_score = 0x00406CD4u + 4u * 12u;
    int limit = (int8_t)rd8(m, last_score + 4u);
    int count = limit < 1 ? 1 : (limit > 7 ? 7 : limit);
    uint32_t offset = 0u;
    r->d[5] = (rd8(m, last_score + 5u) >> 2) & 1u;
    for (int i = 1; i < count; ++i)
        offset += (uint32_t)i + r->d[5];
    r->d[4] = (uint32_t)((count - 9) * 2);
    r->a[1] = 0x0013D5C8u;
    r->a[2] = r->a[1] + 2u * offset;
}

uint32_t Seg_0B8404(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    const uint32_t a5 = r->a[5];
    switch (pc) {
    case 0x0B8404u:
        /* 0x0B842E: 0x14DA loads both palette banks before the clears.
         * 0x5A12 and 0x117E save and restore the registers they use. */
        palette_bank_registers(m, r, 0x00016890u, 0x00016900u);
        palette_bank_registers(m, r, 0x000168C0u, 0x0001FB40u);
        Gfx_ClearAllLayers(m);
        Sprite_ClearAll(m);
        return 0x0B8408u;

    case 0x0B840Au: {
        uint8_t notice = rd8(m, a5 + 0x089Au);
        wr8(m, a5 + 0x089Au, (uint8_t)(notice | 1u));
        if (!(notice & 1u)) {
            seg_push32(m, r, 0x0B8418u);
            return Seg_Run(m, r, 0x0B8442u);
        }
    }
        /* fall through */
    case 0x0B8418u:
        seg_push32(m, r, 0x0B841Cu);
        return Seg_Run(m, r, 0x0B8956u);
    case 0x0B841Cu:
        seg_push32(m, r, 0x0B8420u);
        return Seg_Run(m, r, 0x0B89EAu);
    case 0x0B8420u:
        /* 0x0B8E2C is a single RTS. */
    case 0x0B8424u:
        seg_push32(m, r, 0x0B8428u);
        return Seg_Run(m, r, 0x0B8E7Au);
    case 0x0B8428u:
        seg_push32(m, r, 0x0B842Cu);
        return Seg_Run(m, r, 0x0B8F7Cu);
    case 0x0B842Cu:
        return 0x0B8408u;

    case 0x0B8442u: {
        uint16_t region = rd16(m, 0x001FFFFEu);
        if (region == 3u)
            return Seg_Run(m, r, 0x0B86A4u);

        /* 0x0B845A / 0x0B8562: each ROM string occupies 0x1F bytes. */
        uint32_t src = region == 2u ? 0x0B87E2u : 0x0B86ACu;
        uint32_t dst = region == 2u ? 0x0061C18Cu : 0x0061C28Cu;
        unsigned lines = region == 2u ? 12u : 10u;
        for (unsigned i = 0; i < lines; ++i) {
            r->d[0] = Text_DrawString(m, src, dst, 7u);
            src += 0x1Fu;
            dst += 0x100u;
        }
        return sleep_at(m, r, 0x0B86A0u, 0xB4u);
    }
    case 0x0B86A2u:
        r->a[7] += 4u;
        /* fall through */
    case 0x0B86A4u:
        Gfx_ClearAllLayers(m);
        return Seg_Run(m, r, seg_pop32(m, r));

    case 0x0B8956u: {
        init_line_ram(m, r);
        uint32_t slot = Pal_QueueRamp(m, 0x0000190Eu, 0x000201C0u,
                                      0x700u, 0x3Cu, 0x20u);
        r->d[0] = slot ? slot : 0xFFFFFFFFu;
        return sleep_at(m, r, 0x0B8984u, 2u);
    }
    case 0x0B8986u:
        r->a[7] += 4u;
        Gfx_DrawCompressedTilemap(m, 0x00066FACu, 0x00610118u, 0u); /* preserves registers */
        return sleep_at(m, r, 0x0B89A8u, 0x78u);
    case 0x0B89AAu: {
        r->a[7] += 4u;
        uint32_t slot = Pal_QueueRamp(m, 0x000201C0u, 0x0000190Eu,
                                      0x700u, 0x3Cu, 0x20u);
        r->d[0] = slot ? slot : 0xFFFFFFFFu;
        return sleep_at(m, r, 0x0B89D4u, 0x3Cu);
    }
    case 0x0B89D6u:
        r->a[7] += 4u;
        Gfx_ClearAllLayers(m);
        return sleep_at(m, r, 0x0B89E4u, 0x3Cu);
    case 0x0B89E6u:
        r->a[7] += 4u;
        return Seg_Run(m, r, seg_pop32(m, r));

    case 0x0B89EAu:
        if (rd16(m, 0x001FFFFEu) == 2u) {
            init_line_ram(m, r);
            Gfx_ClearAllLayers(m);
            Gfx_DrawCompressedTilemap(m, 0x00066DDEu, 0x00610000u, 0u);
            return sleep_at(m, r, 0x0B8A22u, 0x78u);
        }
        return Seg_Run(m, r, 0x0B8A26u);
    case 0x0B8A24u:
        r->a[7] += 4u;
        /* fall through */
    case 0x0B8A26u:
        Gfx_ClearAllLayers(m);
        spawn_task(m, r, 2u, 0x0B935Au); /* 0x0B8A38 */
        return sleep_at(m, r, 0x0B8A42u, 0x0C40u);
    case 0x0B8A44u: {
        r->a[7] += 4u;
        uint16_t region = rd16(m, 0x001FFFFEu);
        uint32_t src = region == 3u ? 0x0B8DE0u : (region == 2u ? 0x0B8DBAu : 0x0B8D94u);
        r->d[0] = Text_DrawString(m, src, 0x0061CC84u, 7u);
        r->d[0] = Text_DrawString(m, 0x0B8E06u, 0x0061CD04u, 7u);
        return sleep_at(m, r, 0x0B8AD4u, 0x12Cu);
    }
    case 0x0B8AD6u:
        r->a[7] += 4u;
        Gfx_ClearAllLayers(m);
        palette_bank_registers(m, r, 0x00016890u, 0x00016900u);
        return Seg_Run(m, r, seg_pop32(m, r));

    case 0x0B8E7Au:
        seg_push32(m, r, 0x0B8E80u);
        return Seg_Run(m, r, 0x0B8FC8u);
    case 0x0B8E80u:
        seg_setw(&r->d[1], 0x0Cu);
        return sleep_at(m, r, 0x0B8E8Au, 0x1Eu);
    case 0x0B8E8Cu:
        r->a[7] += 4u;
        r->d[0] = Text_DrawString(m, 0x0B8F4Cu, 0x0061C71Cu, 8u);
        return sleep_at(m, r, 0x0B8EAEu, 0x1Eu);
    case 0x0B8EB0u:
        r->a[7] += 4u;
        r->d[0] = Text_DrawString(m, 0x0B8F70u, 0x0061C71Cu, 8u);
        return sleep_at(m, r, 0x0B8ED2u, 0x1Eu);
    case 0x0B8ED4u: {
        r->a[7] += 4u;
        uint32_t src = rd8(m, a5 - 0x5DC1u) ? 0x0B8F64u : 0x0B8F58u;
        r->d[0] = Text_DrawString(m, src, 0x0061C71Cu, 7u);
        return sleep_at(m, r, 0x0B8F1Cu, 0x1Eu);
    }
    case 0x0B8F1Eu:
        r->a[7] += 4u;
        r->d[0] = Text_DrawString(m, 0x0B8F70u, 0x0061C71Cu, 8u);
        if (dbra(&r->d[1]))
            return sleep_at(m, r, 0x0B8E8Au, 0x1Eu);
        Sound_StopAll(m); /* 0x3C24 preserves its registers */
        wr8(m, a5 - 0x144Cu, (uint8_t)(rd8(m, a5 - 0x144Cu) | 2u));
        return Seg_Run(m, r, seg_pop32(m, r));

    case 0x0B8F7Cu:
        /* This JSR reuses the prompt's sleep argument slot. */
        seg_push32(m, r, 0x0B8F82u);
        draw_high_scores(m, r);
        r->a[7] += 4u;
        seg_setw(&r->d[7], 0x12Bu);
        return 0x0B8F88u;
    case 0x0B8F8Au: {
        if (dbra(&r->d[7]))
            return 0x0B8F88u;

        wr8(m, a5 - 0x144Cu, (uint8_t)(rd8(m, a5 - 0x144Cu) & ~2u));
        /* TRAP #2 at 0x0B8F9A preserves slot 0 and the current task. */
        uint32_t self = 1u << (rd16(m, a5 - 0x1906u) & 31u);
        uint32_t kill = 0x7FFCu & ~(self | 1u);
        wr32(m, a5 - 0x1904u, rd32(m, a5 - 0x1904u) & ~kill);
        Gfx_ClearAllLayers(m);
        Sprite_ClearAll(m);
        Pal_QueueUpload(m, 0x0001FB40u, 0u, 1u); /* 0x1660 preserves its registers */
        return Seg_Run(m, r, seg_pop32(m, r));
    }

    case 0x0B8FC8u: {
        wr8(m, a5 + 0x08A0u, 0x81u);
        wr16(m, a5 + 0x1D38u, 0u);
        wr32(m, a5 + 0x087Au, 0x00800000u);
        wr32(m, a5 + 0x087Eu, 0x00800000u);
        uint32_t roll = Rng_NextMem(m) & 0xFFFFu;
        r->d[0] = ((roll / 6u) << 16) | (roll % 6u);
        wr16(m, a5 + 0x32DCu, (uint16_t)r->d[0]);
        Leaf_13CAAE(m, r);
        spawn_task(m, r, 2u, 0x144B60u);
        spawn_task(m, r, 3u, 0x0C63E8u);
        spawn_task(m, r, 5u, 0x0C6A64u);
        spawn_task(m, r, 6u, 0x13F2C6u);
        spawn_task(m, r, 4u, 0x010488u);
        spawn_task(m, r, 0xEu, 0x010502u);
        return 0x0B9060u;
    }
    case 0x0B9062u:
        r->d[0] = Obj_Spawn(m, r->a[0], 0x11A87Eu, 0xBu, 1u, 0u);
        return Seg_Run(m, r, seg_pop32(m, r));

    case 0x0B8408u:
    case 0x0B86A0u:
    case 0x0B8984u:
    case 0x0B89A8u:
    case 0x0B89D4u:
    case 0x0B89E4u:
    case 0x0B8A22u:
    case 0x0B8A42u:
    case 0x0B8AD4u:
    case 0x0B8E8Au:
    case 0x0B8EAEu:
    case 0x0B8ED2u:
    case 0x0B8F1Cu:
    case 0x0B8F88u:
    case 0x0B9060u:
        return pc;
    }
    return SEG_LEAVE;
}
