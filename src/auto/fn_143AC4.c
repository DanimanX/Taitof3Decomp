#include "fn_auto.h"
/* ROM 0x143AC4: redraw the test-mode object inspector and its three sprite previews. */
#include "round8_projectile_regs.h"
#include "fn_text_frames.h"

/* 0x143B56 and the row handlers use the same word-sized text addressing. */
static uint32_t row_address(f3regs_t *r)
{
    r->d[4] = set_word(r->d[4], (uint16_t)(r->d[4] << 7));
    uint16_t offset = (uint16_t)(r->d[3] * 2u + r->d[4]);
    r->d[3] = 0x61C000u + (uint32_t)(int32_t)(int16_t)offset;
    return r->d[3];
}

static void row_hex(const f3mem_t *m, f3regs_t *r, uint32_t ret, uint32_t value,
                    uint16_t digits)
{
    r->d[3] = set_word(r->d[3], (uint16_t)(r->d[3] + 5u));
    push_long(m, r, r->d[4]);
    push_long(m, r, r->d[3]);
    uint32_t address = row_address(r);
    draw_hex(m, r, ret, value, digits, address, 7u);
    r->d[3] = rd32(m, r->a[7]);
    r->d[4] = rd32(m, r->a[7] + 4u);
    r->a[7] += 8u;
}

/* Each label in the ROM table at 0x143BDA is followed by a row callback. */
static void draw_row_value(const f3mem_t *m, f3regs_t *r)
{
    switch (r->a[2]) {
    case 0x143C02u: {
        uint32_t delta = rd32(m, r->a[5] + 0x3320u) - 0x40C2B4u;
        uint32_t quotient = delta / 8u;
        r->d[0] = quotient > 0xFFFFu ? delta : ((delta & 7u) << 16) | quotient;
        row_hex(m, r, 0x143C36u, r->d[0], 3u);
        break;
    }
    case 0x143C46u: {
        r->a[2] = rd32(m, r->a[5] + 0x3320u);
        r->d[0] = set_word(r->d[0], rd16(m, r->a[2]));
        row_hex(m, r, 0x143C72u, r->d[0], 4u);
        r->d[0] += 2u;
        r->d[2] = set_word(r->d[2], rd16(m, r->a[5] + 0x330Cu));
        draw_hex(m, r, 0x143C92u, r->d[2], 4u, r->d[0], 7u);
        break;
    }
    case 0x143CA4u:
        r->a[2] = rd32(m, r->a[5] + 0x3320u);
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, r->a[2] + 6u);
        row_hex(m, r, 0x143CD6u, r->d[0], 2u);
        break;
    case 0x143CE2u:
        r->a[2] = rd32(m, r->a[5] + 0x3320u);
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, r->a[2] + 7u);
        row_hex(m, r, 0x143D14u, r->d[0], 2u);
        break;
    case 0x143D20u: /* rts */
        break;
    }
}

/* ROM 0x14482A: show the selected item's name and sprite, then restore its fields. */
static void draw_item(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_long(m, r, 0x143BCAu);
    push_long(m, r, rd32(m, obj + 0x10u));                        /* 0x14482A */
    push_word(m, r, rd16(m, obj + 0x1Cu));
    push_word(m, r, rd16(m, obj + 0x1Eu));
    push_long(m, r, r->a[1]);

    r->d[0] = (r->d[0] & 0xFFu) * 12u;
    uint32_t item = 0x703Eu + r->d[0];                            /* 0x14483C */
    wr32(m, obj + 0x10u, rd32(m, item));
    wr16(m, obj + 0x1Cu, rd16(m, item + 4u));
    wr16(m, obj + 0x1Eu, rd16(m, item + 6u));
    r->a[1] = item + 8u;
    draw_text(m, r, 0x14486Eu, r->a[1], 0x61C504u, 10u);
    wr16(m, obj + 0x14u, 8u);
    wr16(m, obj + 0x18u, 0x50u);
    draw_projectile(m, r, 0x144884u);

    r->a[1] = rd32(m, r->a[7]);
    wr16(m, obj + 0x1Eu, rd16(m, r->a[7] + 4u));
    wr16(m, obj + 0x1Cu, rd16(m, r->a[7] + 6u));
    wr32(m, obj + 0x10u, rd32(m, r->a[7] + 8u));
    r->a[7] += 16u;
}

int Fn_143AC4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t record = r->a[5] + 0x3320u;
    r->a[1] = rd32(m, record);
    r->d[1] = set_word(r->d[1], rd16(m, r->a[1] + 2u));
    draw_text(m, r, 0x143AE2u, 0x143BCEu, 0x61C402u, 5u);
    draw_hex(m, r, 0x143AF8u, r->d[1], 4u, r->d[0], 7u);
    r->d[1] = set_word(r->d[1], rd16(m, r->a[1] + 4u));
    draw_text(m, r, 0x143B16u, 0x143BD4u, 0x61C482u, 5u);
    draw_hex(m, r, 0x143B2Cu, r->d[1], 4u, r->d[0], 7u);

    r->d[1] = set_word(r->d[1], 0u);
    r->a[1] = 0x143BDAu;
    do {                                                         /* 0x143B38 */
        r->d[2] = rd32(m, r->a[1]);
        r->a[1] += 4u;
        r->d[3] = set_word(r->d[3], 1u);
        r->d[4] = set_word(r->d[4], (uint16_t)(r->d[1] + 3u));
        uint16_t attr = (uint16_t)r->d[1] == rd16(m, r->a[5] + 0x331Eu) ? 8u : 3u;
        r->d[0] = set_word(r->d[0], attr);
        push_long(m, r, r->d[4]);
        push_long(m, r, r->d[3]);
        uint32_t address = row_address(r);
        draw_text(m, r, 0x143B74u, r->d[2], address, attr);
        r->d[3] = rd32(m, r->a[7]);
        r->d[4] = rd32(m, r->a[7] + 4u);
        r->a[7] += 8u;
        r->a[2] = rd32(m, r->a[1]);
        r->a[1] += 4u;
        push_long(m, r, 0x143B80u);
        draw_row_value(m, r);
        r->a[7] += 4u;
        r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] + 1u));
    } while ((int16_t)r->d[1] < 5);

    r->a[1] = rd32(m, record);                                    /* 0x143B88 */
    wr16(m, obj + 0x14u, rd16(m, r->a[1] + 2u));
    wr16(m, obj + 0x18u, rd16(m, r->a[1] + 4u));
    wr8(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    draw_projectile(m, r, 0x143BAAu);
    wr8(m, obj + 0x1Cu, 3u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 1u));
    draw_projectile(m, r, 0x143BBEu);
    r->a[1] = rd32(m, record);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, r->a[1] + 7u);
    draw_item(m, r);
    wr32(m, obj, 0x143AC4u);                                     /* 0x143BCA -> 0x143ABC */
    r->a[7] += 4u;
    return 0;
}
