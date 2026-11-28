#include "fn_auto.h"
/* ROM 0x12A290: edit camera diagnostics and redraw the debug overlay. */
#include "fn_call_frames.h"
#include "../video/sprite.h"

typedef uint32_t (*draw_number_t)(const f3mem_t *, uint32_t, uint16_t,
                                  uint32_t, uint16_t);

/* ROM 0x0056E0: draw the requested bits from high to low as L or H. */
static uint32_t draw_bits(const f3mem_t *m, uint32_t value, uint16_t count,
                          uint32_t dst, uint16_t attr)
{
    while ((int16_t)--count >= 0) {
        wr8(m, dst++, (uint8_t)(attr * 2u));
        wr8(m, dst++, (value & (1u << (count & 31u))) ? 0x48u : 0x4Cu);
    }
    return dst;
}

/* ROM 0x005690, 0x0056E0 and 0x00571A return only D0. */
static void call_draw(const f3mem_t *m, f3regs_t *r, uint32_t ret,
                      draw_number_t draw, uint32_t value, uint16_t count,
                      uint32_t dst, uint16_t attr)
{
    uint32_t sp = r->a[7];
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_word(m, r, count);
    push_long(m, r, value);
    push_long(m, r, ret);
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[0]);
    if (draw == Gfx_DrawHex || draw == draw_bits)
        push_long(m, r, r->d[7]);
    if (draw == Gfx_DrawDecimal) {
        push_long(m, r, r->d[5]);
        push_long(m, r, r->d[4]);
    }
    if (draw != draw_bits)
        push_long(m, r, r->d[3]);
    push_long(m, r, r->d[2]);
    push_long(m, r, r->d[1]);
    r->d[0] = draw(m, value, count, dst, attr);
    r->a[7] = sp;
}

/* ROM 0x12A420: print six words as four hexadecimal digits each. */
static void draw_camera_words(const f3mem_t *m, f3regs_t *r)
{
    static const uint32_t dst[] = {
        0x61C384u, 0x61C390u, 0x61C39Cu, 0x61C404u, 0x61C410u, 0x61C41Cu
    };
    for (uint32_t i = 0; i < 6u; ++i) {
        r->d[0] = set_word(r->d[0], rd16(m, r->a[5] + 0x3282u + i * 2u));
        call_draw(m, r, 0x12A43Au + i * 0x12u, Gfx_DrawHex,
                  r->d[0], 4u, dst[i], 7u);
    }
    r->a[7] += 4u;
}

/* ROM 0x12A4D6: active-low buttons adjust the two camera offsets. */
static void move_camera(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    for (unsigned bit = 0; bit < 4u; ++bit) {
        if (!(rd8(m, a5 - 0x5DD8u) & (1u << bit))) {
            uint32_t word = a5 + ((bit < 2) ? 0x30D6u : 0x30D2u);
            wr16(m, word, (uint16_t)(rd16(m, word) + ((bit & 1u) ? 1u : -1u)));
        }
    }
    r->a[7] += 4u;
}

/* 0x12A33C and 0x12A384 use signed bit offsets from the memory operand. */
static uint32_t bit_address(uint32_t base, uint32_t offset)
{
    return base + (uint32_t)((int32_t)offset >> 3);
}

int Fn_12A290(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5], flags = a5 + 0x30EEu, row = a5 + 0x30F2u;
    uint32_t selection = a5 + 0x329Au, buttons = a5 - 0x5DD7u;
    wr16(m, a5 + 0x3318u, 0);
    if (!(rd8(m, a5 - 0x5DD8u) & 0x40u)) {
        push_long(m, r, 0x12A2A2u);
        move_camera(m, r);
        goto rearm;
    }

    /* 0x12A2A4–0x12A2EC: wrap the row with byte arithmetic. */
    if (!(rd8(m, buttons) & 0x20u))
        wr8(m, row, (uint8_t)(rd8(m, row) + 1u));
    if (!(rd8(m, buttons) & 0x10u))
        wr8(m, row, (uint8_t)(rd8(m, row) - 1u));
    if ((int8_t)rd8(m, row) < 0) {
        wr8(m, row, (uint8_t)(rd8(m, row) + 8u));
        wr8(m, row, rd8(m, row)); /* SUBI.B #0 */
        wr8(m, row, (uint8_t)(rd8(m, row) + 1u));
    }
    if ((int8_t)rd8(m, row) > 8) {
        wr8(m, row, (uint8_t)(rd8(m, row) - 8u));
        wr8(m, row, rd8(m, row)); /* ADDI.B #0 */
        wr8(m, row, (uint8_t)(rd8(m, row) - 1u));
    }
    wr8(m, flags + 1u, (uint8_t)(rd8(m, flags + 1u) & ~0x04u));

    /* 0x12A2F6–0x12A324: move the flag selection with button repeat. */
    r->d[0] = rd8(m, selection);
    push_long(m, r, r->a[0]);
    push_word(m, r, 1u);
    push_word(m, r, 14u);
    push_word(m, r, 0u);
    push_word(m, r, (uint16_t)r->d[0]);
    r->d[0] = (uint16_t)(int16_t)(int8_t)rd8(m, a5 - 0x5DD8u);
    push_word(m, r, (uint16_t)r->d[0]);
    push_long(m, r, 0x12A31Cu);
    Fn_19FFF0(m, r);
    r->a[7] += 10u;
    r->a[0] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    wr8(m, selection, (uint8_t)r->d[0]);

    if ((rd8(m, buttons) & 3u) != 3u) {
        uint32_t byte = bit_address(flags, r->d[0]);
        wr8(m, byte, (uint8_t)(rd8(m, byte) ^ (0x80u >> (r->d[0] & 7u))));
    }

    r->d[0] = rd16(m, flags) >> 1;
    call_draw(m, r, 0x12A35Eu, draw_bits, r->d[0], 15u, 0x61C184u, 7u);
    r->d[0] += 2u;
    r->d[1] = (r->d[1] & 0xFFFFFF00u) | rd8(m, row);
    call_draw(m, r, 0x12A37Au, Gfx_DrawHex, r->d[1], 2u, r->d[0], 7u);

    r->d[0] = rd8(m, selection);
    r->d[2] = (rd8(m, bit_address(flags, r->d[0])) >> (7u - (r->d[0] & 7u))) & 1u;
    r->d[0] += 2u;
    r->d[1] = set_word(r->d[1], 3u);
    push_long(m, r, r->d[1]);
    push_long(m, r, r->d[0]);
    r->d[1] = set_word(r->d[1], 0x180u);
    r->d[0] = 0x61C000u + (int16_t)(r->d[0] * 2u + 0x180u);
    call_draw(m, r, 0x12A3B4u, draw_bits, r->d[2], 1u, r->d[0], 8u);
    r->d[0] = rd32(m, r->a[7]);
    r->d[1] = rd32(m, r->a[7] + 4u);
    r->a[7] += 8u;

    r->d[0] = set_word(r->d[0], rd16(m, a5 + 0x30D2u));
    call_draw(m, r, 0x12A3D6u, Gfx_DrawHex, r->d[0], 4u, 0x61C204u, 7u);
    r->d[0] = set_word(r->d[0], rd16(m, a5 + 0x30D6u));
    call_draw(m, r, 0x12A3F4u, Gfx_DrawHex, r->d[0], 4u, 0x61C20Eu, 7u);
    r->d[0] = rd16(m, a5 + 0x30DAu);
    call_draw(m, r, 0x12A414u, Gfx_DrawDecimal, r->d[0], 4u, 0x61C218u, 7u);
    push_long(m, r, 0x12A41Cu);
    draw_camera_words(m, r);

rearm: /* 0x12A288 */
    wr32(m, r->a[0], 0x12A290u);
    r->a[7] += 4u;
    return 0;
}
