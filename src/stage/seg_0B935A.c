/* ROM 0x0B935A: attract-mode stage 2 background and stage-clear banner. */
#include "../kernel/seg.h"
#include "stage_timeline.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_1194C2.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/palette.h"
#include "../video/sprite.h"
#include <stddef.h>

#define FRAME       (F3_A5 + 0x3384u)
#define HANDLER     (F3_A5 + 0x3386u)
#define HANDLER_TBL (F3_A5 + 0x338Au)
#define BLEND_TBL   (F3_A5 + 0x3BAEu)
#define EVENT_TBL   (F3_A5 + 0x3BB6u)
#define STAGE_CLEAR (F3_A5 + 0x3BD6u)
#define INITIAL_EVENTS   0x000BE2B8u
#define INITIAL_HANDLERS 0x000BE39Cu

/* 0x1E76 TRAP #1, specialized to the fixed slot 4 and entry at 0x10488.
 * The caller supplies only two arguments, so the child's argument is the
 * long already at the parent's SP. */
static void spawn_object_task(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t slot = 4u;
    uint32_t mask = rd32(m, F3_A5 - 0x1904u);
    if (mask & (1u << slot)) {
        r->d[0] = 0u;
        return;
    }
    r->d[0] = slot;
    wr32(m, F3_A5 - 0x1904u, mask | (1u << slot));
    wr16(m, F3_A5 - 0x1878u + 2u * slot, 0xFFFFu);
    wr32(m, F3_A5 - 0x18F8u + 4u * slot,
         (uint32_t)rd16(m, F3_A5 - 0x1906u) << 16);

    uint32_t sp = rd32(m, 0x2368u + 4u * slot) - 0x4Cu;
    for (unsigned i = 0; i < 8u; i++)
        wr32(m, sp + 4u * i, r->d[i]);
    for (unsigned i = 0; i < 7u; i++)
        wr32(m, sp + 0x20u + 4u * i, r->a[i]);
    wr16(m, sp + 0x3Cu, 0x2000u);
    wr32(m, sp + 0x3Eu, 0x00010488u);
    wr16(m, sp + 0x42u, 0u);
    wr32(m, sp + 0x44u, 0x00001E48u);
    wr32(m, sp + 0x48u, rd32(m, r->a[7]));
    wr32(m, F3_A5 - 0x1948u + 4u * slot, sp);
}

static void start_effect(const f3mem_t *m, f3regs_t *r, uint8_t id, unsigned bank)
{
    unsigned count = bank ? 8u : 6u;
    r->a[0] = bank ? 0x000168C0u : 0x00016890u;
    r->d[0] = ((uint32_t)(id / count) << 16) | (id % count);
    r->d[1] = rd32(m, r->a[0] + 8u * (id % count) + 4u);
    if ((int32_t)r->d[1] > 0)
        seg_setw(&r->d[1], (uint16_t)(r->d[1] + 1u));
    if (bank)
        Fx_StartB(m, id);
    else
        Fx_StartA(m, id);
}

static void init_line_ram(const f3mem_t *m, f3regs_t *r)
{
    /* 0x14DA calls both bank loaders. Only D2/D3/A2 survive 0x15EC
     * and 0x5B0C. The loaders copy to palette RAM at 0x440000, outside
     * the f3mem address space. Bank 1 consumes six bytes per descriptor. */
    uint32_t bank = 0x000168C0u;
    r->a[2] = rd32(m, bank) + 6u * (rd32(m, bank + 4u) + 1u);
    seg_setw(&r->d[2], 0xFFFFu);
    start_effect(m, r, 7u, 1u);

    Gfx_InitLineRam(m);
    r->a[0] = 0x00005C38u;
    r->a[1] = 0x00621000u;
    r->d[0] = 0x00180000u;
    seg_setw(&r->d[0], rd16(m, rd16(m, F3_A5 - 0x5D58u)
                               ? 0x00624600u : 0x006247FEu));
    seg_setw(&r->d[1], 0xFFFFu);
    r->d[3] = 0x0000FFFFu;
    /* The eight ror.l #4 at 0x5B70 leave D2 as it was after 0x14DA. */
}

/* 0x183C/0x1866/0x1890/0x18BA all restore their scratch registers
 * and return the slot address (or -1) in D0. */
static void queue_ramp(const f3mem_t *m, f3regs_t *r, uint32_t src, uint32_t dst,
                       uint16_t offset, uint16_t frames, uint16_t count)
{
    uint32_t slot = Pal_QueueRamp(m, src, dst, offset, frames, count);
    r->d[0] = slot ? slot : 0xFFFFFFFFu;
}

/* 0x16AB2C advances both address registers by one long. */
static void write_scroll_position(const f3mem_t *m, f3regs_t *r,
                                  unsigned layer, uint32_t x, uint32_t y)
{
    r->a[0] = F3_A5 + 0x4204u + 8u * layer;
    r->a[1] = F3_A5 - 0x5DA8u + 8u * layer;
    r->d[0] = x;
    r->d[1] = y;
    Mem_WritePair64(m, r->a[0], r->a[1], x, y);
    r->a[0] += 4u;
    r->a[1] += 4u;
}

static void fill_line_words(const f3mem_t *m, uint32_t base, uint16_t value)
{
    uint32_t start = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (unsigned row = 0; row < 0xE8u; row++)
        wr16(m, start + 2u * row, value);
}

/* 0x0B9BCC: install stage 2 tilemaps and seed its four scroll layers.
 * The tilemap and layer-clear callees restore their scratch registers. */
static void install_background(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned x = 0; x < 4u; x++) {
        for (unsigned y = 0; y < 4u; y++) {
            uint32_t src = (y & 1u) ? 0x0006766Au : 0x00067590u;
            Gfx_DrawCompressedTilemap(m, src, 0x00613000u + 0x20u * x + 0x400u * y, 0u);
        }
    }
    Gfx_ClearPf1AndLineRam(m);
    Gfx_DrawCompressedTilemap(m, 0x00090956u, 0x00612218u, 0u);
    Fn_16A310(m, r);
    wr16(m, F3_A5 + 0x4294u, 0xFFFFu);  /* 0x16AB62 */
    wr32(m, F3_A5 + 0x4284u, 0u);       /* four 0x16AB58 calls */
    wr32(m, F3_A5 + 0x4224u, 0u);       /* four 0x16AB26 calls */
    wr32(m, F3_A5 + 0x4228u, 0u);
    wr32(m, F3_A5 + 0x422Cu, 0u);
    wr32(m, F3_A5 + 0x4230u, 0u);
    wr32(m, F3_A5 + 0x4234u, 0u);
    wr32(m, F3_A5 + 0x4238u, 0x3000u);
    wr32(m, F3_A5 + 0x423Cu, 0x0C00u);
    wr32(m, F3_A5 + 0x4240u, 0x1800u);
    write_scroll_position(m, r, 2u, 0u, 0xFFCA0000u);
}

/* 0x145484: one frame-keyed blend-table entry. */
static void step_blend(const f3mem_t *m, f3regs_t *r, int16_t frame)
{
    r->a[0] = rd32(m, BLEND_TBL);
    uint16_t key = rd16(m, r->a[0]);
    seg_setw(&r->d[0], key);
    if ((int16_t)key > frame)
        return;

    uint16_t value = rd16(m, r->a[0] + 2u);
    seg_setw(&r->d[0], value);
    seg_setw(&r->d[1], 8u);
    unsigned blend = value & 0xFu;
    if (blend > 8u)
        blend = 8u;
    fill_line_words(m, 0x00626200u, (uint16_t)~(0x8000u | (blend << 8) | (8u - blend)));
    wr32(m, BLEND_TBL, r->a[0] + 4u);
}

typedef struct {
    uint32_t pc;
    uint32_t object;
} frame_event_t;

static const frame_event_t *find_frame_event(uint32_t pc)
{
    static const frame_event_t events[] = {
        { 0x0B9BCCu, 0u },
        { 0x0B9E58u, 0x0BE248u },
        { 0x0BB530u, 0x0BB572u },
        { 0x0B9E80u, 0x0BD248u },
        { 0x0B9F4Au, 0u },
        { 0x0B9F9Cu, 0x0BB654u },
        { 0x0B9EA0u, 0x0BD398u },
        { 0x0BA010u, 0u },
        { 0x0B9EE4u, 0x0BD47Eu },
        { 0x0B9F04u, 0x0BD630u },
        { 0x0B9F2Au, 0x0BD8C0u },
        { 0x0BA0A6u, 0u },
    };
    for (unsigned i = 0; i < sizeof events / sizeof events[0]; i++) {
        if (events[i].pc == pc)
            return &events[i];
    }
    return NULL;
}

static void rebuild_background(const f3mem_t *m, f3regs_t *r);

static void run_frame_event(const f3mem_t *m, f3regs_t *r, const frame_event_t *event)
{
    uint32_t arg = 0u;
    switch (event->pc) {
    case 0x0B9BCCu:
        install_background(m, r);
        return;
    case 0x0B9E58u:
        start_effect(m, r, 5u, 0u);
        arg = 0x0BE534u;
        break;
    case 0x0BB530u:
        queue_ramp(m, r, 0x190Eu, 0x1E080u, 0x6B80u, 0x4Bu, 0x20u);
        break;
    case 0x0B9F4Au:
        queue_ramp(m, r, 0x26940u, 0x267C0u, 0x1000u, 0x60u, 0x20u);
        queue_ramp(m, r, 0xBA026u, 0x20340u, 0x0880u, 0x60u, 0x20u);
        return;
    case 0x0B9F9Cu:
        r->a[0] = F3_A5 + 0x422Cu;
        r->d[1] = 0x8000u;
        wr32(m, r->a[0], 0u);
        wr32(m, r->a[0] + 4u, r->d[1]);
        Pal_QueueUpload(m, 0x26D40u, 0x1400u, 0x20u);
        Gfx_DrawCompressedTilemap(m, 0x9098Eu, 0x611058u, 0u);
        break;
    case 0x0B9EA0u:
        Gfx_ClearTextLayer(m);
        Obj_Spawn(m, r->a[0], 0x0BD282u, 0u, 1u, 0u);
        break;
    case 0x0BA010u:
        Obj_Free(m, rd32(m, F3_A5 + 0x08ACu));
        Gfx_ClearPf0b(m);
        return;
    case 0x0B9F04u:
        Gfx_ClearPf2AndLineRam(m);
        break;
    case 0x0BA0A6u:
        rebuild_background(m, r);
        return;
    }
    r->d[0] = Obj_Spawn(m, r->a[0], event->object, 0u, 1u, arg);
    if (event->pc == 0x0B9F9Cu)
        wr32(m, F3_A5 + 0x08ACu, r->d[0]);
}

/* 0x145638 runs every due event. 0x145464 latches at most one new handler
 * and then runs the latched callback. Check both before mutating the frame. */
static int background_supported(const f3mem_t *m, int16_t frame, int initializing)
{
    uint32_t event = initializing ? INITIAL_EVENTS : rd32(m, EVENT_TBL);
    uint32_t script = initializing ? INITIAL_HANDLERS : rd32(m, HANDLER_TBL);
    uint32_t flags = rd32(m, F3_A5 + 0x4284u);
    while ((int16_t)rd16(m, event) <= frame) {
        uint32_t target = rd32(m, event + 2u);
        if (!find_frame_event(target))
            return 0;
        if (target == 0x0B9BCCu)
            flags = 0u;
        event += 6u;
    }
    uint32_t handler = initializing ? rd32(m, script + 4u) : rd32(m, HANDLER);
    if ((int16_t)rd16(m, script) <= frame)
        handler = rd32(m, script + 2u);
    if (handler == 0x0B9B84u)
        flags &= 0x00FFFFFFu;
    if (flags & 0x04040404u)
        return 0;                   /* 0x16A3E0 tile-column streaming registers */
    switch (handler) {
    case 0x0B934Eu: case 0x0B9358u:
    case 0x0B984Au: case 0x0B9854u:
    case 0x0B9948u:
    case 0x0B9B7Au: case 0x0B9B84u:
        return 1;
    default:
        return 0;                   /* unmodelled recurring handler, e.g. 0x0B9964 */
    }
}

static void run_latched_handler(const f3mem_t *m, f3regs_t *r)
{
    switch (r->a[0]) {
    case 0x0B934Eu:
    case 0x0B9358u:
        wr32(m, HANDLER, 0x0B9358u);
        break;
    case 0x0B9854u:
        seg_setw(&r->d[0], rd16(m, FRAME) & 0x7Fu);
        if ((uint16_t)r->d[0] == 0u) {
            queue_ramp(m, r, 0x26D94u, 0x190Eu, 0x1454u, 0x3Fu, 4u);
            queue_ramp(m, r, 0x26DA4u, 0x190Eu, 0x1464u, 0x1Fu, 6u);
        }
        if ((uint16_t)r->d[0] == 0x20u)
            queue_ramp(m, r, 0x190Eu, 0x26DA4u, 0x1464u, 0x10u, 6u);
        if ((uint16_t)r->d[0] == 0x40u) {
            queue_ramp(m, r, 0x190Eu, 0x26D94u, 0x1454u, 0x3Fu, 4u);
            queue_ramp(m, r, 0x26DA4u, 0x190Eu, 0x1464u, 0x1Fu, 6u);
        }
        if ((uint16_t)r->d[0] == 0x60u)
            queue_ramp(m, r, 0x190Eu, 0x26DA4u, 0x1464u, 0x1Fu, 6u);
        /* 0x0B9944 branches to the same re-arm as the 0x0B984A entry. */
        /* fall through */
    case 0x0B984Au:
        wr32(m, HANDLER, 0x0B9854u);
        break;
    case 0x0B9948u:
        wr32(m, F3_A5 + 0x3BBAu, 0x7FFFFFu - rd32(m, F3_A5 - 0x5D94u));
        wr32(m, HANDLER, 0x0B9964u);
        break;
    case 0x0B9B84u:
        wr8(m, F3_A5 + 0x4284u, 0u);
        write_scroll_position(m, r, 0u, 0u, 0u);
        r->a[0] = F3_A5 + 0x4224u;
        wr32(m, r->a[0], 0u);
        wr32(m, r->a[0] + 4u, 0u);
        /* fall through */
    case 0x0B9B7Au:
        wr32(m, HANDLER, 0x0B9B84u);
        break;
    }
}

static uint32_t background_frame(const f3mem_t *m, f3regs_t *r)
{
    int16_t frame = (int16_t)rd16(m, FRAME);
    step_blend(m, r, frame);

    uint32_t event = rd32(m, EVENT_TBL);
    r->a[0] = event;
    seg_setw(&r->d[0], rd16(m, event));
    while ((int16_t)r->d[0] <= frame) {
        r->a[0] = rd32(m, event + 2u);
        run_frame_event(m, r, find_frame_event(r->a[0]));
        event = rd32(m, EVENT_TBL) + 6u;
        wr32(m, EVENT_TBL, event);
        r->a[0] = event;
        seg_setw(&r->d[0], rd16(m, event));
    }

    uint32_t script = rd32(m, HANDLER_TBL);
    seg_setw(&r->d[0], rd16(m, script));
    if ((int16_t)r->d[0] <= frame) {
        wr32(m, HANDLER, rd32(m, script + 2u));
        wr32(m, HANDLER_TBL, script + 6u);
    }
    r->a[0] = rd32(m, HANDLER);
    run_latched_handler(m, r);

    /* 0x16A3E0 with tile streaming disabled. Reloading the descriptor
     * clobbers A0, and the last enabled parallax axis clobbers D1. */
    int reload = rd16(m, F3_A5 + 0x4294u) != 0xFFFFu &&
                 rd16(m, F3_A5 + 0x4292u) == 0u;
    StageScroll_Step(m);
    if (reload)
        r->a[0] = rd32(m, F3_A5 + 0x4288u);
    for (unsigned layer = 1; layer < 4u; layer++) {
        if (rd8(m, F3_A5 + 0x4284u + layer) & 2u)
            r->d[1] = (uint32_t)((int32_t)rd32(m, F3_A5 + 0x4228u) >> 8);
    }
    r->d[0] = rd32(m, F3_A5 + 0x4240u);
    if (rd8(m, F3_A5 + 0x4287u) & 2u)
        r->d[0] = (uint32_t)((int32_t)(int16_t)(r->d[0] >> 8) * (int16_t)r->d[1]);
    return 0x000B9496u;
}

static uint32_t start_stage_clear(const f3mem_t *m, f3regs_t *r)
{
    Gfx_ClearPf2AndLineRam(m);       /* 0x59B4 restores D0-D2/A0-A1 */
    Fn_16A310(m, r);
    write_scroll_position(m, r, 2u, 0u, 0x00080000u);
    write_scroll_position(m, r, 1u, 0x00080000u, 0xFFFC0000u);
    queue_ramp(m, r, 0x0000198Eu, 0x000207C0u, 0x0D00u, 0x7Fu, 0x20u);
    queue_ramp(m, r, 0x0000190Eu, 0x00020140u, 0x0680u, 0x7Fu, 0x20u);
    Sound_PlayCmdForced(m, Sound_PreCmdHook(m, 0x43));
    Sound_PlayCmdForced(m, Sound_PreCmdHook(m, 0x2F));
    Sound_PlayHooked(m, 0x14u);
    return 0x000B956Au;
}

static uint32_t line_page_offset(const f3mem_t *m)
{
    return (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
}

/* 0x1768CA, specialized to a constant step. */
static void gradient_position(const f3mem_t *m, uint32_t base, int start, int end,
                              uint32_t acc, uint32_t step)
{
    base += line_page_offset(m);
    int direction = start <= end ? 1 : -1;
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {
        acc = 0u - acc;
        step = 0u - step;
    }
    for (int row = start; row != end + direction; row += direction) {
        uint32_t addr = base + (uint32_t)(2 * row);
        wr16(m, addr, (uint16_t)((rd16(m, addr) & 0xFE00u) | ((acc >> 16) & 0x1FFu)));
        acc += step;
    }
    wr32(m, F3_A5 + 0x4298u, acc);
}

/* 68k DIVS.W leaves the dividend intact if the quotient overflows. */
static uint16_t divs_word_quotient(int32_t dividend, int16_t divisor)
{
    int32_t quotient = dividend / divisor;
    return (uint16_t)(quotient < -32768 || quotient > 32767 ? dividend : quotient);
}

/* 0x176832, specialized to a zero initial accumulator and constant step.
 * Returns A0. D0-D7/A1 are restored. */
static uint32_t gradient_shade(const f3mem_t *m, uint32_t shade_base, uint32_t scale_base,
                               int start, int end, uint32_t step, uint16_t level)
{
    uint32_t offset = line_page_offset(m);
    shade_base += offset;
    scale_base += offset;
    uint32_t acc = 0u;
    int direction = start <= end ? 1 : -1;
    for (int row = start; row != end + direction; row += direction) {
        int16_t divisor = (int16_t)(0xA0u + (acc >> 16));
        uint16_t scale = (uint16_t)(divs_word_quotient(-0xA000, divisor) + 0x100u);
        int32_t brightness = (0x24 - level) * (int16_t)scale * 64;
        uint16_t shade = (uint16_t)(divs_word_quotient(brightness, 0x32) ^ 0x3Fu);
        uint32_t off = (uint32_t)(2 * row);
        wr16(m, shade_base + off, shade);
        wr8(m, scale_base + off, (uint8_t)scale);
        acc += step;
    }
    wr32(m, F3_A5 + 0x4298u, acc);
    wr16(m, F3_A5 + 0x42A0u, level);
    return shade_base;
}

/* 0x0BA0A6: redraw the background and reset the layer 1 and 3 scroll. */
static void rebuild_background(const f3mem_t *m, f3regs_t *r)
{
    static const uint32_t tilemaps[] = {
        0x6D7C4u, 0x6DC7Eu, 0x6DA3Eu, 0x6D846u, 0x6DD04u, 0x6DB04u,
        0x6D8F2u, 0x6DD6Cu, 0x6DA3Eu, 0x6D8F2u, 0x6DDC2u, 0x6DB04u,
    };
    static const struct { uint32_t src; uint16_t dst; } palettes[] = {
        { 0x22040u, 0x1500u }, { 0x22740u, 0x1C00u }, { 0x22340u, 0x1800u },
        { 0x226C0u, 0x1B80u }, { 0x22640u, 0x1B00u }, { 0x224C0u, 0x1980u },
        { 0x220C0u, 0x1580u }, { 0x219C0u, 0x0E80u }, { 0x223C0u, 0x1880u },
        { 0x21940u, 0x0E00u }, { 0x22840u, 0x1D00u }, { 0x22240u, 0x1700u },
    };
    fill_line_words(m, 0x62B200u, 0x3007u);
    gradient_shade(m, 0x62A200u, 0x628200u, 0, 0xE7, 0u, 1u);
    gradient_shade(m, 0x62A600u, 0x628600u, 0, 0xE7, 0u, 3u);
    gradient_position(m, 0x624600u, 0, 0xE7, 0u, 0u);
    wr32(m, F3_A5 + 0x422Cu, 0u);
    wr32(m, F3_A5 + 0x4230u, 0u);
    wr32(m, F3_A5 + 0x423Cu, 0u);
    wr32(m, F3_A5 + 0x4240u, 0u);
    write_scroll_position(m, r, 1u, 0x00800000u, 0xFF400000u);
    write_scroll_position(m, r, 3u, 0x00800000u, 0u);
    for (unsigned i = 0; i < sizeof tilemaps / sizeof tilemaps[0]; i++)
        Gfx_DrawCompressedTilemap(m, tilemaps[i],
                                  0x613000u + (i / 3u) * 0x20u + (i % 3u) * 0x400u, 0u);
    for (unsigned i = 0; i < sizeof palettes / sizeof palettes[0]; i++)
        Pal_QueueUpload(m, palettes[i].src, palettes[i].dst, 0x20u);
}

/* 0x0B975E: paint both sides of the moving stage-clear boundary. */
static void fade_banner(const f3mem_t *m, f3regs_t *r)
{
    uint16_t phase = (uint16_t)(0x7Fu - rd16(m, FRAME));
    uint16_t row = (uint16_t)(2u * phase);
    gradient_position(m, 0x624400u, 0, (int16_t)row, (uint32_t)row << 16, 0xFFFF0000u);
    r->d[1] = (uint32_t)((int16_t)rd16(m, 0x00027E40u + 2u * (phase & 0xFFu)) * 0x20);
    gradient_shade(m, 0x62A400u, 0x628400u, (int16_t)row, 0, r->d[1], 2u);
    seg_setw(&r->d[0], (uint16_t)(row + 1u));
    gradient_position(m, 0x624400u, (int16_t)r->d[0], 0xE7, 0u, 0u);
    r->a[0] = gradient_shade(m, 0x62A400u, 0x628400u, (int16_t)r->d[0], 0xE7, 0u, 2u);
}

/* 0x0B968C calls 0x53B2, which restores D0-D7/A0-A2. */
static void draw_banner(const f3mem_t *m)
{
    static const uint32_t src[] = {
        0x00067A32u, 0x00067A3Au, 0x00067A48u, 0x00067A50u, 0x00067A5Eu, 0x00067A66u
    };
    Gfx_DrawCompressedTilemap(m, 0x000679FCu, 0x00612084u, 0u);
    Gfx_DrawCompressedTilemap(m, 0x00067A74u, 0x0061231Cu, 0u);
    for (unsigned i = 0; i < sizeof src / sizeof src[0]; i++)
        Gfx_DrawCompressedTilemap(m, src[i],
                                  0x00613000u + (i / 2u) * 0x20u + (i % 2u) * 0x400u, 0u);
}

uint32_t Seg_0B935A(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    switch (pc) {
    case 0x000B935Au:
        wr16(m, FRAME, 0u);
        spawn_object_task(m, r);
        init_line_ram(m, r);
        seg_push32(m, r, 5u);        /* 0x0B9382: argument for TRAP #4 */
        return 0x000B9388u;

    case 0x000B938Au:
        r->a[7] += 4u;
        queue_ramp(m, r, 0x0000190Eu, 0x000BA026u, 0x0880u, 0x96u, 0x20u);
        queue_ramp(m, r, 0x0000190Eu, 0x00026940u, 0x1000u, 0x96u, 0x20u);
        /* The last JSR and its argument overwrite the old sleep argument. */
        wr16(m, r->a[7] - 2u, 0x42u);
        wr32(m, r->a[7] - 6u, 0x000B93E0u);
        Sound_PlayHooked(m, 0x42u);
        return 0x000B93E4u;

    case 0x000B93E6u:
        if (!background_supported(m, (int16_t)rd16(m, FRAME), 1))
            return SEG_LEAVE;
        Fn_16A310(m, r);
        wr16(m, STAGE_CLEAR, 0u);
        fill_line_words(m, 0x0062B000u, 0x300Cu);
        fill_line_words(m, 0x0062B200u, 0x300Au);
        fill_line_words(m, 0x0062B400u, 0x3008u);
        fill_line_words(m, 0x0062B600u, 0x3006u);
        fill_line_words(m, 0x00626200u, 0xBFB7u);
        seg_setw(&r->d[0], 0u);
        seg_setw(&r->d[1], 0xBFB7u);
        seg_setw(&r->d[7], 0xFFFFu);
        wr32(m, BLEND_TBL, 0x000BE3CCu);
        wr32(m, EVENT_TBL, INITIAL_EVENTS);
        wr32(m, HANDLER_TBL, INITIAL_HANDLERS);
        wr32(m, HANDLER, rd32(m, INITIAL_HANDLERS + 4u));
        return background_frame(m, r);

    case 0x000B9498u: {
        uint16_t frame = (uint16_t)(rd16(m, FRAME) + 1u);
        int stage_clear = rd16(m, STAGE_CLEAR) != 0u;
        if (!stage_clear && !background_supported(m, (int16_t)frame, 0))
            return SEG_LEAVE;
        wr16(m, FRAME, frame);
        return stage_clear ? start_stage_clear(m, r) : background_frame(m, r);
    }

    case 0x000B956Cu:
        wr16(m, FRAME, 0u);
        seg_setw(&r->d[0], rd16(m, F3_A5 - 0x1456u) & 0xFCu);
        wr16(m, F3_A5 + 0x3BBAu, (uint16_t)r->d[0]);
        fade_banner(m, r);
        return 0x000B958Au;

    case 0x000B958Cu: {
        uint16_t frame = rd16(m, FRAME);
        if (frame == 1u)
            draw_banner(m);
        if (frame == 0x3Fu)
            queue_ramp(m, r, 0x000BE434u, 0x000207C0u, 0x0D00u, 0x40u, 0x20u);
        fade_banner(m, r);
        wr16(m, FRAME, ++frame);
        if (frame != 0x7Fu)
            return 0x000B958Au;

        Gfx_DrawCompressedTilemap(m, 0x00066F9Cu, 0x0061151Cu, 0u);
        Gfx_DrawCompressedTilemap(m, 0x00067A86u, 0x0061123Cu, 0u);
        if (rd16(m, 0x001FFFFEu) != 1u)
            r->d[0] = Text_DrawString(m, 0x000B9682u, 0x0061C63Eu, 8u);
        return 0x000B9630u;
    }

    case 0x000B9632u: {
        uint16_t frame = rd16(m, FRAME);
        if (frame != 0x8Fu) {
            wr16(m, FRAME, (uint16_t)(frame + 1u));
            return 0x000B9630u;
        }
        queue_ramp(m, r, 0x000BE4B4u, 0x000207C0u, 0x0D00u, 0x40u, 0x20u);
        Sound_PlayHooked(m, 0x4Eu);
        return 0x000B967Eu;          /* TRAP #3, before the kernel kills this task */
    }

    case 0x000B9388u:
    case 0x000B93E4u:
    case 0x000B9496u:
    case 0x000B956Au:
    case 0x000B958Au:
    case 0x000B9630u:
    case 0x000B967Eu:
        return pc;

    case 0x000B9680u:
        return Seg_Run(m, r, seg_pop32(m, r));
    }
    return SEG_LEAVE;
}
