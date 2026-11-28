#include "fn_auto.h"
/* ROM 0x121E1E: edit two objects with active-low controls and redraw their diagnostics. */
#include "fn_call_frames.h"
#include "../video/sprite.h"

/* ROM 0x0056E0: draw bits high to low as L or H. */
static uint32_t draw_bits(const f3mem_t *m, uint32_t value, uint16_t count,
                          uint32_t dst, uint16_t attr)
{
    while ((int16_t)--count >= 0) {
        wr8(m, dst++, (uint8_t)(attr * 2u));
        wr8(m, dst++, (value & (1u << (count & 31u))) ? 0x48u : 0x4Cu);
    }
    return dst;
}

static void call_draw(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                      uint32_t (*draw)(const f3mem_t *, uint32_t, uint16_t,
                                       uint32_t, uint16_t),
                      uint32_t value, uint16_t count, uint32_t dst, uint16_t attr)
{
    uint32_t sp = r->a[7];
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_word(m, r, count);
    push_long(m, r, value);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[0]);
    push_long(m, r, r->d[7]);
    if (draw == Gfx_DrawHex)
        push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[0] = draw(m, value, count, dst, attr);
    r->a[7] = sp;
}

/* Captured diagnostic slots can point into program ROM. The F3 bus
 * ignores the coordinate and flag stores in that address range. */
static void adjust_word(const f3mem_t *m, uint32_t addr, int delta)
{
    if ((addr & 0xFFFFFFu) >= F3_ROM_SIZE)
        wr16(m, addr, (uint16_t)(rd16(m, addr) + delta));
}

/* The second object repeats 0x121E22–0x121F2E at 0x121F32–0x12203E. */
static void edit_object(const f3mem_t *m, f3regs_t *r, unsigned player)
{
    uint32_t a5 = r->a[5];
    uint32_t obj = rd32(m, a5 + 0x3264u + 4u * player);
    uint32_t input = a5 - 0x5DDCu + 4u * player;
    uint32_t text = 0x61C184u + 0x100u * player;
    uint32_t pc = 0x110u * player;
    uint32_t selection = a5 + 0x326Cu + player;
    r->a[1] = obj;
    if (!(rd8(m, input) & 8u))
        adjust_word(m, obj + 0x20u, 0x100);
    if (!(rd8(m, input) & 4u))
        adjust_word(m, obj + 0x20u, -0x100);
    if (!(rd8(m, input) & 1u))
        adjust_word(m, obj + 0x3Eu, 0x80);
    if (!(rd8(m, input) & 2u))
        adjust_word(m, obj + 0x3Eu, -0x80);

    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x20u));
    call_draw(m, r, 0x121E80u + pc, Gfx_DrawHex, r->d[0], 4u, text, 7u);
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x3Eu));
    call_draw(m, r, 0x121E9Eu + pc, Gfx_DrawHex, r->d[0], 4u, text + 0xAu, 7u);
    if (!(rd8(m, input + 1u) & 0x10u))
        wr8(m, selection, (uint8_t)(rd8(m, selection) + 1u));
    if (!(rd8(m, input + 1u) & 0x20u))
        wr8(m, selection, (uint8_t)(rd8(m, selection) - 1u));
    wr8(m, selection, rd8(m, selection) & 7u);
    if (!(rd8(m, input + 1u) & 0x40u)) {
        r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, selection);
        if (((obj + 0x58u) & 0xFFFFFFu) >= F3_ROM_SIZE)
            wr8(m, obj + 0x58u, (uint8_t)(rd8(m, obj + 0x58u) ^ (1u << (r->d[0] & 7u))));
    }
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | rd8(m, obj + 0x58u);
    call_draw(m, r, 0x121EF0u + pc, draw_bits, r->d[0], 8u, text + 0x80u, 7u);

    unsigned offset = 7u - rd8(m, selection);
    r->d[2] = (rd8(m, obj + 0x58u) >> (7u - offset)) & 1u;
    r->d[0] = offset + 2u;
    r->d[1] = set_word(r->d[1], (uint16_t)(4u + 2u * player));
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);
    r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] << 7));
    r->d[0] = 0x61C000u + (uint16_t)(2u * r->d[0] + r->d[1]);
    call_draw(m, r, 0x121F2Au + pc, draw_bits, r->d[2], 1u, r->d[0], 8u);
    r->d[0] = rd32(m, r->a[7]);
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->a[7] += 8u;
}

int Fn_121E1E(const f3mem_t *m, f3regs_t *r)
{
    wr16(m, r->a[5] + 0x3318u, 0u);
    edit_object(m, r, 0u);
    edit_object(m, r, 1u);
    wr32(m, r->a[0], 0x121E1Eu); /* 0x122042 branches to 0x121E16. */
    r->a[7] += 4u;
    return 0;
}
