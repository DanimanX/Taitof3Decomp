#include "fn_auto.h"
/* ROM 0x0910CC: UNVERIFIED (no capture): initialize and run the object-checker menu task. */
#include "fn_text_frames.h"
#include "../kernel/kernel.h"

#define RECORDS 0x41A260u
#define TEXT 0x61C000u

enum {
    AFTER_INIT = 0x091152,
    MAIN_MENU = 0x0912BE,
    RECORD_EDITOR = 0x091622,
    SUB_MENU = 0x091D84,
    AFTER_SUB_MENU = 0x091DE0
};

static uint32_t text_position(uint16_t x, uint16_t y)
{
    return TEXT + (uint32_t)(int32_t)(int16_t)(uint16_t)(2u * x + (y << 7));
}

static uint32_t pop_long(const f3mem_t *m, f3regs_t *r)
{
    uint32_t value = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return value;
}

/* 0x0B724A: PC extensions at 0x0B724C and 0x0B7262 both select
 * 0x0B7292. A0 ends at the start of the glyphs after the second LEA. */
static void load_glyphs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t src = 0x0B7292u;
    uint32_t dst = 0x61E040u;
    for (unsigned i = 0; i < 40u; ++i, src += 4u, dst += 4u)
        wr32(m, dst, rd32(m, src));
    uint32_t fill = 0x11111111u;
    for (unsigned glyph = 0; glyph < 8u; ++glyph, fill += 0x11111111u)
        for (unsigned i = 0; i < 8u; ++i, dst += 4u)
            wr32(m, dst, fill);
    r->a[0] = 0x0B7292u;
    r->a[1] = dst;
    r->d[0] = set_word(r->d[0], 0xFFFFu);
    r->d[1] = fill;
}

/* 0x091E2C / 0x0014E8 / 0x001520: all working registers are saved.
 * Table entries are {pointer.l, DBF counter.l}; each block header has
 * a longword count followed by a single source/destination word pair. */
static void reload_palette(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_long(m, r, ret);
    for (unsigned i = 5u; i > 0u;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 8u; i > 0u;)
        push_long(m, r, r->d[--i]);
    push_long(m, r, 0x091E3Cu);

    uint8_t bank = (uint8_t)(1u + rd8(m, r->a[5] - 0xC4Eu));
    uint32_t entry = 0x016890u + 8u * (bank % 6u);
    uint32_t block = rd32(m, entry);
    uint32_t counter = rd32(m, entry + 4u);
    if ((int32_t)counter >= 0) {
        for (uint32_t i = 0; i <= (uint16_t)counter; ++i, block += 6u) {
            uint32_t count = 1u + (uint16_t)(rd16(m, block) - 1u);
            uint32_t src = 0x016900u + rd16(m, block + 2u);
            uint16_t dst = rd16(m, block + 4u);
            for (uint32_t j = 0; j < count; ++j, src += 4u, dst += 4u)
                wr32(m, 0x440000u + (int16_t)dst, rd32(m, src));
        }
    }
    r->a[7] = sp;
}

/* 0x0B7332 preserves D0-D5/A0-A1. */
static void frame_box(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                      uint16_t w, uint16_t h)
{
    uint32_t sp = r->a[7];
    push_long(m, r, r->a[0]);
    push_word(m, r, 5u);
    push_word(m, r, h);
    push_word(m, r, w);
    push_long(m, r, ret);
    uint32_t entry = r->a[7];
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    for (unsigned i = 6u; i > 0u;)
        push_long(m, r, r->d[--i]);
    r->a[7] = entry;
    (void)Fn_0B7332(m, r);
    r->a[7] = sp;
}

/* 0x0B73E2: signed ADDA.W offsets are added separately. The two
 * dimensions are DBF counts, and A0/A1 and D0-D3 are restored. */
static void clear_box(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                      uint16_t w, uint16_t h)
{
    uint32_t sp = r->a[7];
    uint32_t rect = r->a[0];
    push_long(m, r, rect);
    push_word(m, r, h);
    push_word(m, r, w);
    push_long(m, r, ret);
    push_long(m, r, r->a[1]);
    push_long(m, r, rect);
    for (unsigned i = 4u; i > 0u;)
        push_long(m, r, r->d[--i]);

    uint32_t dst = TEXT + (uint32_t)(int32_t)(int16_t)(rd16(m, rect) * 2u)
                   + (uint32_t)(int32_t)(int16_t)(rd16(m, rect + 2u) << 7);
    uint32_t rows = 1u + (uint16_t)(h - 1u);
    uint32_t cols = 1u + (uint16_t)(w - 1u);
    for (uint32_t y = 0; y < rows; ++y, dst += 0x80u)
        for (uint32_t x = 0; x < cols; ++x)
            wr16(m, dst + 2u * x, 0);
    r->a[7] = sp;
}

/* 0x0B7420: frame position, line count, colors and menu table. */
static void print_menu(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                       uint32_t table, uint16_t rows)
{
    uint32_t sp = r->a[7];
    push_long(m, r, table);
    push_long(m, r, r->a[0]);
    push_word(m, r, 8u);
    push_word(m, r, 7u);
    push_word(m, r, rows);
    push_long(m, r, ret);
    uint32_t entry = r->a[7];
    for (unsigned i = 3u; i > 0u;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 6u; i > 1u;)
        push_long(m, r, r->d[--i]);
    r->a[7] = entry;
    (void)Fn_0B7420(m, r);
    r->a[7] = sp;
}

/* Common 0x19FFF0 call sequence at 0x0913D4, 0x0914E0 and
 * 0x091660/0x0916B6/0x09170E/0x09176A. */
static void adjust_value(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                         uint16_t max, uint16_t step)
{
    push_long(m, r, r->d[0]);
    push_long(m, r, r->a[0]);
    push_word(m, r, step);
    push_word(m, r, max);
    push_word(m, r, 0);
    push_word(m, r, (uint16_t)r->d[1]);
    r->d[0] = set_word(r->d[0], (uint16_t)(int16_t)(int8_t)rd8(m, r->a[5] - 0x5DDCu));
    push_word(m, r, (uint16_t)r->d[0]);
    push_long(m, r, ret);
    (void)Fn_19FFF0(m, r);
    r->a[7] += 10u;
    r->a[0] = pop_long(m, r);
    r->d[1] = set_word(r->d[1], (uint16_t)r->d[0]);
    r->d[0] = pop_long(m, r);
}

/* 0x0914DC: stored duration is one less than the menu value. */
static void adjust_duration(const f3mem_t *m, f3regs_t *r)
{
    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, r->a[1]) + 1u));
    adjust_value(m, r, 0x091504u, 0x79u, 1u);
    r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] - 1u));
    wr16(m, r->a[1], (uint16_t)r->d[1]);
}

/* 0x091518: positive duration, OFF (zero), or END (negative). */
static void print_duration(const f3mem_t *m, f3regs_t *r)
{
    int16_t duration = (int16_t)r->d[1];
    if (duration > 0)
        draw_decimal(m, r, 0x091534u, r->d[1], 3u, r->d[0], 6u);
    else if (duration == 0)
        draw_text(m, r, 0x09154Cu, 0x0913BAu, r->d[0], 3u);
    else
        draw_text(m, r, 0x091564u, 0x0913BEu, r->d[0], 4u);
}

/* 0x091340: adjust the selected row, optionally copy the previous
 * descriptor and flags, then refresh all fourteen rows. */
static void refresh_records(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[0]);
    r->a[1] = RECORDS;
    uint16_t selected = rd16(m, a5 - 0xC68u);
    r->d[0] = set_word(r->d[0], (uint16_t)(selected - 1u));
    /* SUBQ.W #1 / BLT tests N xor V. */
    if ((int16_t)selected >= 1) {
        r->d[0] = (uint32_t)((int32_t)(int16_t)r->d[0] * 12);
        r->a[1] += (uint32_t)(int32_t)(int16_t)r->d[0];
        push_long(m, r, 0x09135Eu);
        adjust_duration(m, r);
        r->a[7] += 4u;
        if (!(rd8(m, a5 - 0x5DDBu) & 0x20u) &&
            (int16_t)rd16(m, a5 - 0xC68u) > 1) {
            uint32_t row = r->a[1];
            wr32(m, row + 2u, rd32(m, row - 10u));
            wr16(m, row + 10u, rd16(m, row - 2u));
        }
    }
    r->d[1] = set_word(r->d[1], 3u);
    r->d[2] = set_word(r->d[2], 3u);
    r->d[3] = set_word(r->d[3], 13u);
    r->a[0] = RECORDS;
    do {
        r->a[1] = rd32(m, r->a[0] + 2u);
        push_long(m, r, 0x091398u);
        (void)Fn_091466(m, r);
        r->a[7] += 4u;
        push_long(m, r, r->d[1]);
        r->d[1] = rd16(m, r->a[0]);
        push_long(m, r, 0x0913A4u);
        print_duration(m, r);
        r->a[7] += 4u;
        r->d[1] = pop_long(m, r);
        r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] + 1u));
        r->a[0] += 12u;
        r->d[3] = set_word(r->d[3], (uint16_t)(r->d[3] - 1u));
    } while ((uint16_t)r->d[3] != 0xFFFFu);
    r->d[0] = pop_long(m, r);
    r->a[0] = pop_long(m, r);
}

int Fn_0910CC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    push_long(m, r, 0x0910D2u);
    load_glyphs(m, r);
    r->a[7] += 4u;

    uint8_t initialized = rd8(m, a5 - 0xC8Eu);
    wr8(m, a5 - 0xC8Eu, (uint8_t)(initialized | 1u));      /* 0x0910D2 BSET */
    if (!(initialized & 1u)) {
        for (uint32_t i = 0; i < 14u; ++i) {               /* 0x0910E6 DBF */
            uint32_t row = RECORDS + 12u * i;
            wr16(m, row, 0xFFFFu);
            wr32(m, row + 2u, 0x091F96u);
            wr16(m, row + 6u, 0);
            wr16(m, row + 8u, 0);
            wr16(m, row + 10u, 0);
        }
    }
    r->a[0] = 0x41A30Cu;
    r->d[1] = rd32(m, 0x05AA28u);
    for (unsigned i = 0; i < 144u; ++i, r->a[0] += 4u)    /* 0x09111C DBF */
        wr32(m, r->a[0], r->d[1]);
    reload_palette(m, r, 0x091126u);
    draw_text(m, r, 0x09113Cu, 0x091319u, 0x61C012u, 8u);

    uint32_t sp = r->a[7];                                /* 0x091140 TRAP #1 */
    push_long(m, r, 4u);
    push_long(m, r, 0x010488u);
    push_word(m, r, 0x0084u);                             /* vector 33 */
    push_long(m, r, 0x09114Eu);
    r->a[7] -= 2u;                                        /* SR belongs to the scheduler */
    (void)Kern_001E76(m, r);
    r->a[7] = sp;
    return 0;                                            /* 0x091150 TRAP #5 */
}

/* 0x0911A4: draw the status and main menu frames after init or editor exit. */
static void draw_main_menu(const f3mem_t *m, f3regs_t *r)
{
    static const struct {
        uint32_t source;
        uint32_t ret;
        uint16_t color;
    } labels[] = {
        { 0x091332u, 0x0911ECu, 10u },
        { 0x091312u, 0x09121Au,  9u },
        { 0x091304u, 0x091248u, 10u },
        { 0x09130Bu, 0x091276u,  6u }
    };

    r->a[0] = 0x407390u;
    frame_box(m, r, 0x0911BEu, 18u, 10u);
    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, r->a[0]) + 1u));
    r->a[0] += 2u;
    r->d[2] = set_word(r->d[2], rd16(m, r->a[0]));
    r->a[0] += 2u;
    for (unsigned i = 0; i < sizeof labels / sizeof labels[0]; ++i) {
        push_long(m, r, r->d[2]);
        push_long(m, r, r->d[1]);
        r->d[1] = text_position((uint16_t)r->d[1], (uint16_t)r->d[2]);
        r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] << 7));
        draw_text(m, r, labels[i].ret, labels[i].source, r->d[1], labels[i].color);
        r->d[1] = pop_long(m, r);
        r->d[2] = pop_long(m, r);
        r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] + 1u));
    }

    push_long(m, r, 0x091286u);
    (void)Fn_0B7A54(m, r);
    r->a[7] += 4u;
    r->a[0] = rd32(m, r->a[5] - 0xC8Cu);
    frame_box(m, r, 0x09129Eu, 19u, 17u);
    draw_text(m, r, 0x0912B8u, 0x091D16u, 0x61C084u, 10u);
}

/* 0x0913C4: advance/rewind the linked list of sprite descriptors. With
 * fast advance pressed, stay in the loop while the sprite pointer matches. */
static void choose_descriptor(const f3mem_t *m, f3regs_t *r)
{
    uint32_t row = r->a[1];
    r->a[2] = rd32(m, row + 2u);
    r->a[3] = rd32(m, r->a[2]);
    r->a[4] = rd32(m, r->a[2] + 4u);
    r->a[2] += 8u;
    r->d[1] = set_word(r->d[1], 1u);
    for (unsigned i = 5u; i > 0u;)
        push_long(m, r, r->a[--i]);
    adjust_value(m, r, 0x0913F8u, 2u, 1u);
    for (unsigned i = 0; i < 5u; ++i)
        r->a[i] = pop_long(m, r);

    int16_t direction = (int16_t)r->d[1];
    while (direction != 1) {
        uint32_t next = direction < 1 ? r->a[3] : r->a[4];
        if ((int16_t)rd16(m, next) < 0)                   /* 0x091418/0x091426 */
            break;
        wr32(m, row + 2u, next);
        r->a[4] = next;
        if (rd8(m, r->a[5] - 0x5DDCu) & 0x20u)
            break;
        r->d[0] = rd32(m, next + 0xCu);
        r->a[3] = rd32(m, next);
        r->a[4] = rd32(m, next + 4u);
        if (r->d[0] != rd32(m, r->a[2] + 4u))
            break;
    }

    push_long(m, r, row);
    r->a[1] = rd32(m, row + 2u);
    r->d[1] = set_word(r->d[1], (uint16_t)(2u + rd16(m, r->a[5] - 0xC68u)));
    r->d[2] = set_word(r->d[2], 3u);
    push_long(m, r, 0x091460u);
    (void)Fn_091466(m, r);
    r->a[7] += 4u;
    r->a[1] = pop_long(m, r);
}

/* 0x09156A: write the sign and magnitude of the selected axis offset. */
static void print_offset(const f3mem_t *m, f3regs_t *r)
{
    r->d[3] = (uint16_t)r->d[0];
    int negative = (int16_t)r->d[3] < 0;
    uint32_t dst = text_position((uint16_t)r->d[1], (uint16_t)r->d[2]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[1] = dst;
    r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] << 7));
    draw_text(m, r, negative ? 0x091596u : 0x0915C8u,
              negative ? 0x0915EAu : 0x0915E8u, dst, 10u);
    r->d[1] = pop_long(m, r);
    r->d[2] = pop_long(m, r);
    if (negative)
        r->d[3] = (uint16_t)(0u - r->d[3]);
    draw_decimal(m, r, 0x0915E2u, r->d[3], 3u, r->d[0], 10u);
}

/* 0x091626..0x0918A6, after the editor's call to 0x091D28 returns. */
static int edit_record(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t row = r->a[1];
    uint16_t field = rd16(m, a5 - 0xC66u);
    if (field == 0u) {
        push_long(m, r, 0x091634u);
        choose_descriptor(m, r);
        r->a[7] += 4u;
    } else if (field == 1u) {
        push_long(m, r, 0x091642u);
        adjust_duration(m, r);
        r->a[7] += 4u;
        if (!(rd8(m, a5 - 0x5DDBu) & 0x20u))
            wr16(m, row, 0);
    } else if (field >= 2u && field <= 5u) {
        static const struct {
            uint32_t ret;
            uint16_t offset;
            uint16_t max;
            uint16_t step;
        } fields[] = {
            { 0x091684u, 10u,    3u, 1u },
            { 0x0916DAu, 11u, 0xFEu, 2u },
            { 0x091732u,  6u, 0x100u, 1u },
            { 0x09178Eu,  8u, 0x100u, 1u }
        };

        uint32_t address = row + fields[field - 2u].offset;
        int byte_field = field <= 3u;
        uint16_t bias = byte_field ? 0u : 0x80u;
        uint16_t value = byte_field ? rd8(m, address) : rd16(m, address);
        if (byte_field)
            r->d[1] = value;                             /* 0x09165A/0x0916B0 MOVEQ */
        else
            r->d[1] = set_word(r->d[1], (uint16_t)(value + bias));
        adjust_value(m, r, fields[field - 2u].ret,
                     fields[field - 2u].max, fields[field - 2u].step);
        r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] - bias));
        if (byte_field) {
            wr8(m, address, (uint8_t)r->d[1]);
            if (!(rd8(m, a5 - 0x5DDBu) & 0x20u))
                wr8(m, address, 0);
        } else {
            wr16(m, address, (uint16_t)r->d[1]);
            if (!(rd8(m, a5 - 0x5DDBu) & 0x20u))
                wr16(m, address, 0);
        }
    }

    print_menu(m, r, 0x0917CEu, 0x0918A8u, 6u);
    uint32_t rect = r->a[0];
    push_long(m, r, r->d[0]);
    r->d[0] = TEXT + (uint16_t)(2u * (rd16(m, rect) + 14u) +
                               (rd16(m, rect + 2u) << 7));
    r->d[1] = rd16(m, row);
    push_long(m, r, 0x0917F6u);
    print_duration(m, r);
    r->a[7] += 4u;

    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, rect) + 2u));
    r->d[2] = set_word(r->d[2], (uint16_t)(rd16(m, rect + 2u) + 3u));
    r->d[0] = rd8(m, row + 10u);
    /* 0x091808 + 0xD4; extension 0x0400 selects D0.w*4 at 0x09180C. */
    r->a[2] = rd32(m, 0x0918DCu + 4u * r->d[0]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[1] = text_position((uint16_t)r->d[1], (uint16_t)r->d[2]);
    r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] << 7));
    draw_text(m, r, 0x091830u, r->a[2], r->d[1], 7u);
    r->d[1] = pop_long(m, r);
    r->d[2] = pop_long(m, r);

    r->d[1] = set_word(r->d[1], (uint16_t)(rd16(m, rect) + 9u));
    r->d[2] = set_word(r->d[2], (uint16_t)(rd16(m, rect + 2u) + 4u));
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, row + 11u);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[1] = text_position((uint16_t)r->d[1], (uint16_t)r->d[2]);
    r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] << 7));
    draw_hex(m, r, 0x09186Cu, r->d[0], 2u, r->d[1], 10u);
    r->d[1] = pop_long(m, r);
    r->d[2] = pop_long(m, r);

    for (unsigned axis = 0; axis < 2u; ++axis) {
        r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] + 1u));
        r->d[0] = set_word(r->d[0], rd16(m, row + 6u + 2u * axis));
        push_long(m, r, 0x09187Eu + 10u * axis);
        print_offset(m, r);
        r->a[7] += 4u;
    }
    r->d[0] = pop_long(m, r);
    if (r->d[0] == 0u)                                  /* 0x09188E -> 0x091620 */
        return RECORD_EDITOR;
    clear_box(m, r, 0x0918A2u, 14u, 8u);
    r->a[7] += 4u;                                       /* 0x0918A6 RTS -> 0x0912F4 */
    wr32(m, a5 - 0xC8Cu, rd32(m, a5 - 0xC8Cu) - 4u);
    draw_main_menu(m, r);                                /* 0x0912F8 -> 0x0911A4 */
    return MAIN_MENU;
}

/* 0x091D28 saves A0/A1 across its own yields; its BSR return PC selects
 * the main menu (0x0912C2) or record editor (0x091626) continuation. */
static int finish_options(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    if (!(rd8(m, a5 - 0x5DD7u) & 0x10u)) {
        uint32_t mode = a5 - 0xC4Eu;
        wr8(m, mode, (uint8_t)(rd8(m, mode) + 1u));
        if ((int8_t)rd8(m, mode) < 0) {                  /* 0x091DF4 BGE */
            wr8(m, mode, (uint8_t)(rd8(m, mode) + 4u));
            wr8(m, mode, rd8(m, mode));                  /* 0x091DFE SUBI.B #0 */
            wr8(m, mode, (uint8_t)(rd8(m, mode) + 1u));
        }
        if ((int8_t)rd8(m, mode) > 4) {                  /* 0x091E0E BLE */
            wr8(m, mode, (uint8_t)(rd8(m, mode) - 4u));
            wr8(m, mode, rd8(m, mode));                  /* 0x091E18 ADDI.B #0 */
            wr8(m, mode, (uint8_t)(rd8(m, mode) - 1u));
        }
        reload_palette(m, r, 0x091E26u);
    }
    r->a[0] = pop_long(m, r);
    r->a[1] = pop_long(m, r);
    uint32_t resume = pop_long(m, r);                    /* 0x091E2A RTS */

    if (resume == 0x091626u)
        return edit_record(m, r);

    print_menu(m, r, 0x0912DCu, 0x091C9Au, 15u);
    push_long(m, r, 0x0912E4u);
    refresh_records(m, r);
    r->a[7] += 4u;
    if ((int32_t)r->d[0] < 0) {                          /* 0x0912E6 -> 0x0912FC */
        uint32_t flags = a5 - 0x144Cu;
        wr8(m, flags, (uint8_t)(rd8(m, flags) | 1u));
        r->a[7] += 4u;                                   /* 0x091302 RTS */
        return 0;
    }
    if (r->d[0] == 0u)                                  /* 0x0912EA -> 0x0912BC */
        return MAIN_MENU;

    /* The other fourteen main-menu rows all dispatch to 0x0915EC. */
    wr32(m, a5 - 0xC8Cu, rd32(m, a5 - 0xC8Cu) + 4u);
    r->a[0] = r->d[0];
    push_long(m, r, 0x0912F4u);
    r->a[0] = rd32(m, a5 - 0xC8Cu);
    frame_box(m, r, 0x091604u, 14u, 8u);
    wr16(m, a5 - 0xC66u, 0);
    r->d[0] = (uint32_t)((int32_t)(int16_t)(rd16(m, a5 - 0xC68u) - 1u) * 12);
    r->a[1] = RECORDS + (uint32_t)(int32_t)(int16_t)r->d[0];
    return RECORD_EDITOR;                               /* 0x091620 TRAP #5 */
}

/* Resume at the supplied PC. A positive return value is the PC after
 * the next TRAP #5. Zero means the menu exited via RTS at 0x091302. */
int TaskObjectChecker_Frame(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    uint32_t a5 = r->a[5];
    switch (pc) {
    case AFTER_INIT:
        wr16(m, a5 - 0xC62u, 0xA8u);                     /* 0x091152 */
        wr16(m, a5 - 0xC5Eu, 0x38u);
        call_spawn(m, r, 0x091176u, 0x091944u, 0u, 1u, 0u);
        wr16(m, 0x407390u, 1u);
        wr16(m, 0x407392u, 0x12u);
        wr32(m, a5 - 0xC8Cu, 0x407378u);
        wr16(m, a5 - 0xC68u, 0);
        wr16(m, 0x407378u, 1u);
        wr16(m, 0x40737Au, 1u);
        draw_main_menu(m, r);
        return MAIN_MENU;                               /* 0x0912BC TRAP #5 */

    case MAIN_MENU:
    case RECORD_EDITOR:
        push_long(m, r, pc == MAIN_MENU ? 0x0912C2u : 0x091626u);
        push_long(m, r, r->a[1]);
        push_long(m, r, r->a[0]);
        if (rd8(m, a5 - 0x5DDBu) & 0x40u)               /* 0x091D32 -> 0x091DE0 */
            return finish_options(m, r);
        r->a[0] = rd32(m, a5 - 0xC8Cu);
        wr32(m, a5 - 0xC6Cu, rd32(m, r->a[0]));
        wr16(m, r->a[0], 0x14u);
        wr16(m, r->a[0] + 2u, 1u);
        wr16(m, a5 - 0xC64u, 0);
        r->a[0] = rd32(m, a5 - 0xC8Cu);
        frame_box(m, r, 0x091D64u, 11u, 5u);
        draw_text(m, r, 0x091D7Eu, 0x091F74u, 0x61C0AAu, 10u);
        return SUB_MENU;                                /* 0x091D82 TRAP #5 */

    case SUB_MENU:
        print_menu(m, r, 0x091D9Eu, 0x091F58u, 3u);
        push_long(m, r, 0x091DA6u);
        (void)Fn_091E42(m, r);
        r->a[7] += 4u;
        if ((rd8(m, a5 - 0x5DDBu) & 0x20u) && (int32_t)r->d[0] >= 0) {
            /* Every submenu handler is -1 or suppressed to zero. */
            return SUB_MENU;                            /* 0x091DB6 -> 0x091D82 */
        }
        clear_box(m, r, 0x091DD6u, 11u, 5u);
        wr32(m, r->a[0], rd32(m, a5 - 0xC6Cu));
        return AFTER_SUB_MENU;                          /* 0x091DDE TRAP #5 */

    case AFTER_SUB_MENU:
        return finish_options(m, r);
    default:
        return -1;
    }
}
