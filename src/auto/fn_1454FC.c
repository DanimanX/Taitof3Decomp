#include "fn_auto.h"
/* ROM 0x1454FC: activate scroll-timed background handlers and tick the current handler. */
#include "fn_call_frames.h"

static uint16_t divs_word_result(int32_t dividend, int16_t divisor)
{
    int32_t quotient = dividend / divisor;
    if (quotient < INT16_MIN || quotient > INT16_MAX)
        return (uint16_t)dividend;
    return (uint16_t)quotient;
}

/* 0x176832, with the two argument sets used by 0x14BF88. */
static void draw_gradient(const f3mem_t *m, f3regs_t *r, uint16_t first,
                          uint16_t last, uint32_t step, uint32_t ret)
{
    uint32_t sp = r->a[7];
    push_word(m, r, 1u);
    push_long(m, r, 0u);
    push_long(m, r, step);
    push_long(m, r, 0u);
    push_word(m, r, last);
    push_word(m, r, first);
    push_long(m, r, 0x628200u);
    push_long(m, r, 0x62A200u);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    for (unsigned i = 8; i > 0;)
        push_long(m, r, r->d[--i]);

    uint32_t a5 = r->a[5];
    uint32_t offset = (uint32_t)(int32_t)(int16_t)rd16(m, a5 - 0x5D56u);
    r->a[0] = 0x62A200u + offset;
    uint32_t shade = 0x628200u + offset;
    uint32_t phase = a5 + 0x4298u;
    wr32(m, phase, 0u);
    wr16(m, a5 + 0x42A0u, 1u);
    int direction = (int16_t)first <= (int16_t)last ? 1 : -1;
    uint16_t row = first;
    do {
        int16_t divisor = (int16_t)(rd16(m, phase) + 0xA0u);
        uint16_t light = (uint16_t)(divs_word_result(-0xA000, divisor) + 0x100u);
        int32_t span = (int16_t)(0x24u - rd16(m, a5 + 0x42A0u));
        int32_t scaled = (int32_t)((uint32_t)(span * (int16_t)light) << 6);
        uint16_t value = divs_word_result(scaled, 0x32) ^ 0x3Fu;
        uint32_t row_offset = (uint32_t)((int32_t)(int16_t)row * 2);
        wr16(m, r->a[0] + row_offset, value);
        wr8(m, shade + row_offset, (uint8_t)light);
        wr32(m, phase, rd32(m, phase) + step);
        row = (uint16_t)(row + direction);
    } while (direction > 0 ? (int16_t)row <= (int16_t)last
                           : (int16_t)row >= (int16_t)last);
    r->a[7] = sp;
}

static void tick_14BF88(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t scroll = rd16(m, a5 - 0x5D9Cu);
    r->d[0] = set_word(r->d[0], scroll);
    if (scroll != rd16(m, a5 + 0x41F0u)) {
        /* 0x14BF92 NEG.W followed by SUBI.W #$10 and BGE. */
        int32_t first = (int16_t)(uint16_t)(0u - scroll) - 0x10;
        if (first >= 0) {
            r->d[0] = set_word(r->d[0], (uint16_t)first);
            push_long(m, r, r->d[0]);
            draw_gradient(m, r, (uint16_t)first, 0u, 0x15555u, 0x14BFD4u);
            r->a[7] += 4u;
        } else {
            r->d[0] = set_word(r->d[0], 0u);
        }
        draw_gradient(m, r, (uint16_t)r->d[0], 0xE7u, 0u, 0x14C00Au);

        push_long(m, r, 0x14C012u);
        /* 0x14C016 fills the second line-RAM table from row 127 down. */
        r->a[1] = 0x629200u + (uint32_t)(int32_t)(int16_t)rd16(m, a5 - 0x5D56u);
        int16_t height = (int16_t)(uint16_t)(0u - rd16(m, a5 - 0x5D9Cu));
        r->d[1] = set_word(r->d[1], (uint16_t)height);
        for (int row = 0x7F; row >= 0; --row) {
            uint16_t value = 0u;
            if (height > row)
                value = (uint16_t)(((row - height + 0x80) >> 3) + 0x1C);
            r->d[0] = set_word(r->d[0], value);
            wr16(m, r->a[1] + 2u * (uint32_t)row, value);
        }
        r->d[7] = set_word(r->d[7], 0xFFFFu);
        r->a[7] += 4u;
    }
    wr32(m, a5 + 0x3386u, 0x14BF88u);               /* 0x14BF7E */
}

static void tick_14C4A6(const f3mem_t *m, f3regs_t *r)
{
    r->a[0] = r->a[5] + 0x4287u;
    r->d[0] &= 0xFFFFFF00u;
    push_long(m, r, 0x14C4B4u);
    wr8(m, r->a[0], 0u);                            /* 0x16AB58 */
    r->a[7] += 4u;
}

int Fn_1454FC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t entry = rd32(m, a5 + 0x338Au);
    uint32_t threshold = rd32(m, entry);
    int advance = (int32_t)threshold <= (int32_t)rd32(m, a5 - 0x5DA8u);
    uint32_t handler = advance ? rd32(m, entry + 4u) : rd32(m, a5 + 0x3386u);

    /* The 21 stage tables also select re-arm stubs of the form MOVE.L
     * #pc, ($3386,A5); RTS, sometimes reached through a short BRA. */
    int copy_scroll = handler == 0x148684u || handler == 0x15B180u ||
                      handler == 0x160A60u;
    uint32_t stub = handler;
    if (copy_scroll)
        stub += 6u;                                /* MOVE.L ($4228,A5),($3BCA,A5) */
    uint32_t next = 0u;
    int bare_return = 0;
    if (stub <= F3_ROM_SIZE - 2u && !(stub & 1u)) {
        uint16_t op = rd16(m, stub);
        if ((op & 0xFF00u) == 0x6000u && (uint8_t)op != 0u && (uint8_t)op != 0xFFu)
            stub += 2u + (uint32_t)(int32_t)(int8_t)op;
        bare_return = stub <= F3_ROM_SIZE - 2u && rd16(m, stub) == 0x4E75u;
        if (stub <= F3_ROM_SIZE - 10u && rd16(m, stub) == 0x2B7Cu &&
            rd16(m, stub + 6u) == 0x3386u && rd16(m, stub + 8u) == 0x4E75u)
            next = rd32(m, stub + 2u);
    }
    if (next == 0u && !bare_return && handler != 0x14BF88u &&
        handler != 0x14C04Eu && handler != 0x14C464u && handler != 0x14C4A6u)
        return -1;

    r->d[0] = threshold;
    if (advance) {
        wr32(m, a5 + 0x3386u, handler);
        wr32(m, a5 + 0x338Au, entry + 8u);
    }
    r->a[0] = handler;
    push_long(m, r, 0x14551Au);
    if (copy_scroll)
        wr32(m, a5 + 0x3BCAu, rd32(m, a5 + 0x4228u));
    if (next != 0u)
        wr32(m, a5 + 0x3386u, next);
    else if (handler == 0x14BF88u)
        tick_14BF88(m, r);
    else if (handler == 0x14C04Eu)
        Fn_14C04E(m, r);
    else if (handler == 0x14C464u)
        Fn_14C464(m, r);
    else if (handler == 0x14C4A6u)
        tick_14C4A6(m, r);
    r->a[7] += 8u;                                  /* handler RTS, 0x14551A RTS */
    return 0;
}
