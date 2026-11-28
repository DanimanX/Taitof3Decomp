/* Darius Gaiden (Taito F3) — stage timeline step and timed-effect starters.
 * Decompiled from ROM 0x0015D0..0x00161C and 0x13F2D4..0x13F340. */
#include "stage_timeline.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/palette.h"
#include "../video/sprite.h"

static void fx_start(const f3mem_t *m, uint32_t table, uint16_t mode, uint8_t id, uint8_t n)
{
    wr16(m, FX_MODE, mode);
    uint16_t idx = (uint16_t)(id % n);                 /* divu.w + swap */
    int32_t dur = (int32_t)rd32(m, table + 4u + 8u * idx);
    if (dur > 0) {
        wr16(m, FX_TIMER, (uint16_t)(dur + 1));
        wr32(m, FX_PTR, rd32(m, table + 8u * idx));
    }
}

/* ROM 0x0015D0 / 0x0015EC with the register file they leave, for callers that
 * read it: A0 = the block table, D0 = the `divu.w` quotient in the high word and
 * `swap`ed remainder (id % 6 or id % 8) in the low word, D1 = the block's
 * duration long with `addq.w #1` when it is positive. The duration word and the
 * pointer still land in the A5 FX variables. */
static void fx_start_regs(const f3mem_t *m, f3regs_t *r, uint32_t table,
                          uint16_t mode, uint8_t id, uint8_t n)
{
    wr16(m, FX_MODE, mode);
    r->a[0] = table;                                   /* 0x0015D6 lea */
    uint32_t d0 = id & 0xFFu;                          /* 0x0015DC andi.l #$ff */
    d0 = ((d0 / n) << 16) | (d0 % n);                  /* 0x0015E2 divu.w ; 0x0015E6 swap */
    r->d[0] = d0;
    int32_t dur = (int32_t)rd32(m, table + 4u + 8u * (d0 & 0xFFFFu)); /* 0x001608 */
    r->d[1] = (uint32_t)dur;
    if (dur > 0) {                                     /* 0x00160C ble */
        r->d[1] = (uint32_t)((uint32_t)dur & 0xFFFF0000u) | (uint16_t)((uint32_t)dur + 1u); /* 0x001610 */
        wr16(m, FX_TIMER, (uint16_t)r->d[1]);          /* 0x001612 */
        wr32(m, FX_PTR, rd32(m, table + 8u * (d0 & 0xFFFFu))); /* 0x001616 */
    }
}

void Fx_StartA_Regs(const f3mem_t *m, f3regs_t *r, uint8_t id)
{ fx_start_regs(m, r, 0x16890u, 0, id, 6); }

void Fx_StartB_Regs(const f3mem_t *m, f3regs_t *r, uint8_t id)
{ fx_start_regs(m, r, 0x168C0u, 1, id, 8); }

void Fx_StartA(const f3mem_t *m, uint8_t id) { fx_start(m, 0x16890u, 0, id, 6); }
void Fx_StartB(const f3mem_t *m, uint8_t id) { fx_start(m, 0x168C0u, 1, id, 8); }

int StageTimeline_Step(const f3mem_t *m, int *spawned)
{
    if (!rd8(m, TL_FX_STARTED) && (int16_t)rd16(m, FX_TIMER) <= 0) {
        wr8(m, TL_FX_STARTED, 0xFF);
        Fx_StartA(m, 1);
    }
    wr32(m, TL_SCROLL_POS, rd32(m, TL_SCROLL_POS) + rd32(m, TL_SCROLL_SPD));
    uint16_t last = rd16(m, TL_LAST_POS);
    wr16(m, TL_LAST_POS, rd16(m, TL_SCROLL_POS));
    if (rd16(m, TL_SCROLL_POS) == last)
        return 0;
    uint32_t a0 = rd32(m, TL_SCRIPT_PTR);
    for (;;) {
        uint16_t pos = rd16(m, a0);
        a0 += 2;
        if (pos != rd16(m, TL_SCROLL_POS))
            return 0;
        if ((int16_t)rd16(m, a0 + 4) < 0)
            return 1;
        Obj_Spawn(m, a0, WAVE_OBJ_FN, 5, 2, a0);
        if (spawned) (*spawned)++;
        a0 += 6;
        wr32(m, TL_SCRIPT_PTR, a0);
    }
}

/* ROM 0x16AB70: scale layer speed by primary layer speed (both 16.16 >> 8, muls.w) */
static int32_t scale_parallax(int32_t d0, int32_t d1)
{
    return (int32_t)(int16_t)(d0 >> 8) * (int32_t)(int16_t)(d1 >> 8);
}

/* ROM 0x16AB78: load 8-byte scroll segment {vx.l, vy_12_4.w, frames.w} */
static void load_scroll_segment(const f3mem_t *m)
{
    uint32_t a0 = rd32(m, F3_A5 + 0x4288u);
    wr32(m, F3_A5 + 0x4224u, rd32(m, a0));
    int32_t vy = (int32_t)(int16_t)rd16(m, a0 + 4u);
    wr32(m, F3_A5 + 0x4228u, (uint32_t)(vy << 4));
    wr16(m, F3_A5 + 0x4292u, rd16(m, a0 + 6u));
}

/* ROM 0x16AB94: pixel X -> 128px tile-column index */
static int16_t tile_col(int16_t x)
{
    return (int16_t)(x - 0x20) >> 7;
}

/* ROM 0x16ABA2 + Gfx_DrawCompressedTilemap loop: stream a 4-row column at col_x */
static void stream_tile_column(const f3mem_t *m, int16_t col_x, uint32_t map_idx_ptr,
                               uint32_t tile_ptrs, uint32_t vram_base)
{
    for (int16_t row = 3; row >= 0; row--) {
        int16_t idx = (int16_t)((col_x << 2) + row);
        int16_t tile_id = (int16_t)rd16(m, map_idx_ptr + (int32_t)(int16_t)(idx << 1));
        uint32_t src = rd32(m, tile_ptrs + (int32_t)(int16_t)(tile_id << 2));
        int16_t off = (int16_t)(((col_x & 3) << 5) + (row << 10));
        Gfx_DrawCompressedTilemap(m, src, vram_base + (uint32_t)off, 0);
    }
}

static void scroll_layer(const f3mem_t *m, int layer)
{
    uint32_t prev_x_addr   = F3_A5 + 0x41E4u + 8u * (uint32_t)layer;
    uint32_t prev_y_addr   = F3_A5 + 0x41E8u + 8u * (uint32_t)layer;
    uint32_t cur_x_addr    = F3_A5 - 0x5DA8u + 8u * (uint32_t)layer;
    uint32_t cur_y_addr    = F3_A5 - 0x5DA4u + 8u * (uint32_t)layer;
    uint32_t acc_x_addr    = F3_A5 + 0x4204u + 8u * (uint32_t)layer;
    uint32_t acc_y_addr    = F3_A5 + 0x4208u + 8u * (uint32_t)layer;
    uint32_t spd_x_addr    = F3_A5 + 0x4224u + 8u * (uint32_t)layer;
    uint32_t spd_y_addr    = F3_A5 + 0x4228u + 8u * (uint32_t)layer;
    uint32_t max_x_addr    = F3_A5 + 0x425Cu + 2u * (uint32_t)layer;
    uint32_t map_idx_addr  = F3_A5 + 0x4264u + 8u * (uint32_t)layer;
    uint32_t tile_ptrs_adr = F3_A5 + 0x4268u + 8u * (uint32_t)layer;
    uint8_t flags          = rd8(m, F3_A5 + 0x4284u + (uint32_t)layer);
    uint32_t vram_base     = 0x610000u + 0x1000u * (uint32_t)layer;

    wr32(m, prev_x_addr, rd32(m, cur_x_addr));
    wr32(m, prev_y_addr, rd32(m, cur_y_addr));

    int32_t dx = (int32_t)rd32(m, spd_x_addr);
    if (layer > 0 && (flags & 2))
        dx = scale_parallax(dx, (int32_t)rd32(m, F3_A5 + 0x4224u));
    uint32_t acc_x = rd32(m, acc_x_addr) + (uint32_t)dx;
    wr32(m, acc_x_addr, acc_x);
    wr32(m, cur_x_addr, acc_x);

    int32_t dy = (int32_t)rd32(m, spd_y_addr);
    if (layer > 0 && (flags & 2))
        dy = scale_parallax(dy, (int32_t)rd32(m, F3_A5 + 0x4228u));
    uint32_t acc_y = rd32(m, acc_y_addr) + (uint32_t)dy;
    wr32(m, acc_y_addr, acc_y);
    wr32(m, cur_y_addr, acc_y);

    if (!(flags & 4))
        return;

    int16_t cur_x = (int16_t)rd16(m, cur_x_addr);
    int16_t prev_x = (int16_t)rd16(m, prev_x_addr);
    if (cur_x == prev_x)
        return;

    if (cur_x > prev_x) {
        int16_t max_x = (int16_t)rd16(m, max_x_addr);
        if (cur_x >= max_x) {
            wr16(m, acc_x_addr, (uint16_t)max_x);
            wr16(m, cur_x_addr, (uint16_t)max_x);
            return;
        }
        if ((int16_t)(cur_x + 0x40) > max_x)
            return;
        if (tile_col(cur_x) == tile_col(prev_x))
            return;
        stream_tile_column(m, (int16_t)(tile_col(cur_x) + 3),
                           rd32(m, map_idx_addr), rd32(m, tile_ptrs_adr), vram_base);
    } else {
        if (cur_x < 0) {
            wr16(m, acc_x_addr, 0);
            wr16(m, cur_x_addr, 0);
            return;
        }
        if (cur_x < 0x80)
            return;
        if (tile_col(cur_x) == tile_col(prev_x))
            return;
        if ((int16_t)(cur_x - 0x20) < 0)
            return;
        stream_tile_column(m, tile_col(cur_x),
                           rd32(m, map_idx_addr), rd32(m, tile_ptrs_adr), vram_base);
    }
}

void StageScroll_Step(const f3mem_t *m)
{
    if ((int16_t)rd16(m, F3_A5 + 0x4294u) != -1) {
        int16_t timer = (int16_t)(rd16(m, F3_A5 + 0x4292u) - 1);
        wr16(m, F3_A5 + 0x4292u, (uint16_t)timer);
        if (timer == -1) {
            wr32(m, F3_A5 + 0x4288u, rd32(m, F3_A5 + 0x4288u) + 8u);
            load_scroll_segment(m);
            if ((int16_t)rd16(m, F3_A5 + 0x4292u) < 0)
                wr16(m, F3_A5 + 0x4294u, 0xFFFFu);
        }
    }
    for (int layer = 0; layer < 4; layer++)
        scroll_layer(m, layer);
}

/* ROM 0x145638: run frame-keyed background script entries {frame.w, fn.l} */
static int bg_run_frame_events(const f3mem_t *m)
{
    uint16_t frame = rd16(m, F3_A5 + 0x3384u);
    for (;;) {
        uint32_t a0 = rd32(m, F3_A5 + 0x3BB6u);
        if ((int16_t)rd16(m, a0) > (int16_t)frame)
            return 0;
        return -1; /* callback at rd32(m, a0 + 2) not yet dispatched */
    }
}

/* ROM 0x14551C: advance/rewind scroll-keyed line-RAM opacity table A5+0x3BAE */
static void bg_step_line_table(const f3mem_t *m)
{
    uint32_t a0 = rd32(m, F3_A5 + 0x3BAEu);
    int32_t d0 = (int32_t)rd32(m, a0);
    int32_t prev = (int32_t)rd32(m, F3_A5 + 0x41E4u);
    int32_t cur = (int32_t)rd32(m, F3_A5 - 0x5DA8u);
    if (d0 > prev && d0 <= cur) {
        /* 0x626200 line-RAM blend table write (unmodeled in f3mem) */
        wr32(m, F3_A5 + 0x3BAEu, a0 + 6u);
        return;
    }
    int32_t prev_entry = (int32_t)rd32(m, a0 - 6u);
    if (prev_entry <= prev && prev_entry > cur) {
        /* 0x626200 line-RAM blend table write (unmodeled in f3mem) */
        wr32(m, F3_A5 + 0x3BAEu, a0 - 6u);
    }
}

/* ROM 0x14561C: run scroll-keyed background script entries {pos.l, fn.l} */
static int bg_run_scroll_events(const f3mem_t *m)
{
    int32_t cur = (int32_t)rd32(m, F3_A5 - 0x5DA8u);
    for (;;) {
        uint32_t a0 = rd32(m, F3_A5 + 0x3BB2u);
        if ((int32_t)rd32(m, a0) > cur)
            return 0;
        return -1; /* callback at rd32(m, a0 + 4) not yet dispatched */
    }
}

/* ROM 0x145F48: stage 1 periodic palette ramp */
static void stage1_fade_145F48(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd16(m, F3_A5 + 0x3384u) & 0x007Fu);
    if (d0 == 1u) {
        Pal_QueueRamp(m, 0x00146244u, 0x001462C4u, 0x1B00u, 0x30u, 0x10u);
        Pal_QueueRamp(m, 0x001462B8u, 0x00146338u, 0x1B74u, 0x30u, 0x03u);
    } else if (d0 == 0x41u) {
        Pal_QueueRamp(m, 0x001462C4u, 0x00146244u, 0x1B00u, 0x40u, 0x10u);
        Pal_QueueRamp(m, 0x00146338u, 0x001462B8u, 0x1B74u, 0x40u, 0x03u);
    }
}

/* ROM 0x1458D0 / 0x145CCA: stage 1 background tick */
static int stage1_bg_tick(const f3mem_t *m)
{
    if (rd16(m, F3_A5 + 0x3384u) == 0u)
        return -1; /* 0x1458D8 init path not yet decompiled */
    if (bg_run_frame_events(m) < 0)
        return -1;
    bg_step_line_table(m);
    if (bg_run_scroll_events(m) < 0)
        return -1;
    stage1_fade_145F48(m);
    Stage_Fade146006(m);
    return 0;
}

/* ROM 0x144BB6..0x144CF0 (frame body of Task_StageBackground, ROM 0x144B60) */
int StageBg_Step(const f3mem_t *m)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x02u)
        return 0;
    uint16_t st = rd16(m, F3_A5 + 0x3384u);
    if (st < 0x40u)
        return -1; /* 0x144BC6..0x144CDA initial line-RAM fade-in */
    uint32_t fn = rd32(m, F3_A5 + 0x3BDAu);
    if (fn != 0x001458D0u)
        return -1;
    StageScroll_Step(m);
    if (stage1_bg_tick(m) < 0)
        return -1;
    wr16(m, F3_A5 + 0x3384u, (uint16_t)(rd16(m, F3_A5 + 0x3384u) + 1u));
    return 0;
}
