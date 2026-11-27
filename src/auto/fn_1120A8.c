#include "fn_auto.h"

/* ROM 0x1120A8: verified: the capture discriminates (an empty body fails it): free, re-arm or draw the sprite according to its position relative to the stage scroll. */
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

/* ROM 0x001294..0x0013CC: track the register outputs of the grid draw.
 * Obj_DrawSprite performs the corresponding sprite writes. */
static void grid_registers(const f3mem_t *m, f3regs_t *r, uint32_t def)
{
    uint32_t obj = r->a[0];
    uint16_t cols = rd16(m, def);
    uint16_t rows = rd16(m, def + 2u);
    uint8_t flip = rd8(m, obj + 0x1Cu);
    uint8_t colour = rd8(m, obj + 0x1Du);
    uint16_t zoom = rd16(m, obj + 0x1Eu);
    uint16_t row_stride = (uint16_t)(cols * 4u);
    int16_t col_stride = (flip & 2u) ? -4 : 4;
    uint32_t tile = def + 4u;
    uint32_t d2 = flip;
    uint32_t d4 = with_word(r->d[4], (uint16_t)flip << 8);

    /* 0x0012B2 uses D2.w*4 (extension 0x2400) to load a pointer from
     * 0x0012B8. Its four entries are 0x12C8, 0x12D2, 0x12E4, 0x12F2. */
    if (flip & 1u) {
        row_stride = (uint16_t)(0u - row_stride);
        d2 = (uint32_t)((int32_t)(int16_t)(rows - 1u) * (int16_t)row_stride);
        tile -= (uint32_t)(int32_t)(int16_t)d2;
    }
    if (flip & 2u)
        tile += (uint32_t)((int32_t)(int16_t)(cols - 1u) * 4);

    uint16_t x = rd16(m, obj + 0x14u);
    uint16_t d3 = x & 0x0FFFu; /* 0x001328 BFEXTU clears the high word. */
    uint16_t y_step = (uint16_t)((zoom >> 8) - 0x100u);
    uint16_t d5_high = (uint16_t)((zoom & 0xFFu) - 0x101u);
    r->d[5] = ((uint32_t)d5_high << 16) | y_step; /* 0x00130C..0x001322 */
    r->d[6] = with_word(r->d[6], (uint16_t)col_stride);
    r->d[7] = with_word(r->d[7], row_stride);

    for (uint32_t row = 0; row <= (uint16_t)(rows - 1u); ++row) {
        r->a[3] = tile; /* 0x001334 */
        uint16_t y = (uint16_t)(rd16(m, obj + 0x18u) << 4);
        d4 &= ~0x80003000u;
        for (uint32_t col = 0; col <= (uint16_t)(cols - 1u); ++col) {
            if (rd16(m, tile) != 0) {
                d3 = rd16(m, tile + 2u) ^ (uint16_t)d4;
                if (colour != 0)
                    d3 = (d3 & 0xFF00u) | colour;
                if (!(d4 & 0x1000u)) { /* 0x00136E: first tile in a run */
                    d4 = (d4 & ~0x8000u) | 0x80007000u;
                    d3 = y >> 4;
                }
            } else {
                d4 &= ~0x3000u;
            }
            tile += (uint32_t)(int32_t)col_stride;
            y = (uint16_t)(y - y_step);
        }
        tile = r->a[3] + (uint32_t)(int32_t)(int16_t)row_stride;
        if (!(d4 & 0x80000000u))
            d4 |= 0x4000u;
        d4 |= 0x8000u;
        d2 = with_word(d2, y);
    }

    r->a[2] = tile;
    r->d[0] = 0xFFFFu;
    r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0xFFFFu;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
}

/* ROM 0x001282: use the existing draw helper, preserving the ROM outputs. */
static void draw_sprite(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t def = rd32(m, obj + 0x10u);
    if (rd32(m, def) == 0x00010001u) { /* 0x001290 -> 0x0013CE */
        uint16_t attr = rd16(m, def + 6u);
        uint16_t flip_word = rd16(m, obj + 0x1Cu) & 0xFF00u;
        uint8_t colour = rd8(m, obj + 0x1Du);
        attr ^= flip_word;
        if (colour != 0)
            attr = (attr & 0xFF00u) | colour;
        r->d[0] = ((uint32_t)(rd16(m, obj + 0x14u) & 0x0FFFu) << 16) | attr;
        r->d[1] = with_word(r->d[1], flip_word);
        r->a[2] = def;
    } else {
        grid_registers(m, r, def);
    }
    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, r->a[5] - 0x0CA0u);
}

int Fn_1120A8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t scroll = r->a[5] - 0x5D90u; /* 0x1120B0: signed displacement 0xA270 */
    uint16_t x = rd16(m, obj + 0x44u);
    uint16_t near = (uint16_t)(x + 0x70u);
    r->d[0] = with_word(r->d[0], near);
    if ((int16_t)near <= (int16_t)rd16(m, scroll)) {
        uint32_t sp = r->a[7];
        wr32(m, sp - 4u, obj); /* 0x1120DA */
        wr32(m, sp - 8u, 0x1120E2u);
        /* 0x0105EC saves A1, A0 and D0, then restores them. */
        wr32(m, sp - 12u, r->a[1]);
        wr32(m, sp - 16u, obj);
        wr32(m, sp - 20u, r->d[0]);
        Obj_Free(m, rd32(m, sp - 4u));
    } else {
        uint16_t far = (uint16_t)(rd16(m, obj + 0x44u) - 0x1B0u); /* 0x1120B8 */
        r->d[0] = with_word(r->d[0], far);
        if ((int16_t)far < (int16_t)rd16(m, scroll)) {
            r->d[0] = rd32(m, obj + 0x44u) - rd32(m, scroll);
            wr32(m, obj + 0x14u, r->d[0]); /* 0x1120CE */
            wr32(m, r->a[7] - 4u, 0x1120D8u);
            draw_sprite(m, r);
        }
        wr32(m, obj, 0x1120A8u); /* 0x1120A0 */
    }
    r->a[7] += 4u;
    return 0;
}
