#include "fn_auto.h"
/* ROM 0x13A0EE: verified: the capture discriminates (an empty body fails it): animate the warning effect, then draw its beam and register collision. */
#include "../objects/obj_common.h"
#include "../video/sprite.h"

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

static void push_long(const f3mem_t *m, f3regs_t *r, uint32_t value)
{
    r->a[7] -= 4;
    wr32(m, r->a[7], value);
}

/* ROM 0x001282: the sprite helper writes the sprite list. Reproduce the
 * caller-visible scratch registers from the same read-only sprite grid. */
static void draw_sprite(const f3mem_t *m, f3regs_t *r, uint32_t return_addr)
{
    push_long(m, r, return_addr);
    uint32_t obj = r->a[0];
    uint32_t source = rd32(m, obj + 0x10);
    uint16_t columns = rd16(m, source);
    uint16_t rows = rd16(m, source + 2);
    uint8_t colour = rd8(m, obj + 0x1D);
    uint16_t attr = rd16(m, obj + 0x1C) & 0xFF00u;
    uint16_t x = rd16(m, obj + 0x14);
    uint16_t y = rd16(m, obj + 0x18);

    if (columns == 1 && rows == 1) { /* 0x0013CE */
        r->a[2] = source;
        r->d[1] = with_word(r->d[1], attr);
        attr ^= rd16(m, source + 6);
        if (colour != 0)
            attr = (attr & 0xFF00u) | colour;
        r->d[0] = ((uint32_t)(x & 0x0FFFu) << 16) | attr;
    } else {
        /* 0x0012B8: four long pointers, one for each combination of flips. */
        uint8_t flip = rd8(m, obj + 0x1C);
        int16_t tile_step = (flip & 2u) ? -4 : 4;
        int16_t row_step = (int16_t)(columns * 4u);
        uint16_t col = (uint16_t)(columns - 1u);
        uint16_t row = (uint16_t)(rows - 1u);
        r->a[2] = source + 4;
        r->d[2] = flip;
        if (flip & 1u) {
            row_step = (int16_t)(-row_step);
            r->d[2] = (uint32_t)((int16_t)row * row_step);
            r->a[2] -= (uint32_t)(int32_t)(int16_t)r->d[2];
        }
        if (flip & 2u)
            r->a[2] += (uint32_t)((int32_t)(int16_t)col * 4);
        r->d[6] = with_word(r->d[6], (uint16_t)tile_step);
        r->d[7] = with_word(r->d[7], (uint16_t)row_step);

        uint16_t zoom_x = (uint16_t)(rd8(m, obj + 0x1F) - 0x101u);
        uint16_t zoom_y = (uint16_t)(rd8(m, obj + 0x1E) - 0x100u);
        r->d[5] = ((uint32_t)zoom_x << 16) | zoom_y;
        r->d[3] = x & 0x0FFFu;
        r->d[4] = with_word(r->d[4], attr);
        do {
            r->a[3] = r->a[2];
            r->d[2] = with_word(r->d[2], (uint16_t)(y << 4));
            r->d[4] &= ~0x80003000u;
            uint16_t remaining = col;
            do {
                if (rd16(m, r->a[2]) != 0) {
                    r->d[3] = with_word(r->d[3], (uint16_t)(rd16(m, r->a[2] + 2) ^ r->d[4]));
                    if (colour != 0)
                        r->d[3] = (r->d[3] & 0xFFFFFF00u) | colour;
                    if (!(r->d[4] & 0x1000u)) {
                        r->d[4] = (r->d[4] | 0x80007000u) & ~0x8000u;
                        r->d[3] = (uint16_t)r->d[2] >> 4;
                    }
                } else {
                    r->d[4] &= ~0x3000u;
                }
                r->a[2] += (uint32_t)(int32_t)tile_step;
                r->d[2] = with_word(r->d[2], (uint16_t)(r->d[2] - zoom_y));
            } while (remaining-- != 0);
            r->a[2] = r->a[3] + (uint32_t)(int32_t)row_step;
            if (!(r->d[4] & 0x80000000u))
                r->d[4] |= 0x4000u;
            r->d[4] |= 0x8000u;
        } while (row-- != 0);
        r->d[0] = 0xFFFFu;
        r->d[1] = ((uint32_t)col << 16) | 0xFFFFu;
    }

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, r->a[5] - 0x0CA0);
    r->a[7] += 4;
}

/* ROM 0x11B09A: compute projected coordinates, draw if visible, and remove
 * the temporary offsets. ADD.W followed by BLE uses the signed overflow flag. */
static void draw_projected(const f3mem_t *m, f3regs_t *r, uint32_t return_addr)
{
    push_long(m, r, return_addr);
    uint32_t obj = r->a[0];
    uint16_t x = (uint16_t)(rd16(m, obj + 0x50) + rd16(m, obj + 0x54)
                           + rd16(m, r->a[5] + 0x30D2));
    wr16(m, obj + 0x14, x);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x52) + rd16(m, obj + 0x56)
                           + rd16(m, r->a[5] + 0x30D6));
    wr16(m, obj + 0x18, y);
    if ((int16_t)y < 0xE8 && (int16_t)x < 0x140) {
        r->a[1] = rd32(m, obj + 0x10);
        int16_t height = (int16_t)(uint16_t)(rd16(m, r->a[1]) << 4);
        r->a[1] += 2;
        if (height + (int16_t)y > 0) {
            int16_t width = (int16_t)(uint16_t)(rd16(m, r->a[1]) << 4);
            r->a[1] += 2;
            if (width + (int16_t)x > 0)
                draw_sprite(m, r, 0x11B0F0);
        }
    }

    r->d[0] = with_word(r->d[0], rd16(m, obj + 0x54));
    wr16(m, obj + 0x14, (uint16_t)(rd16(m, obj + 0x14) - r->d[0]));
    r->d[0] = with_word(r->d[0], rd16(m, obj + 0x56));
    wr16(m, obj + 0x18, (uint16_t)(rd16(m, obj + 0x18) - r->d[0]));
    r->a[7] += 4;
}

int Fn_13A0EE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, obj + 8);
    if (rd8(m, r->a[1] + 0x59) != 0) {
        FnAuto_FreeObject(m, r, obj, 0x13A23A);
        return 0;
    }

    /* 0x13A0FA: offset 21 is bit 2 of the third byte at A5+0x30EE. */
    if (!(rd8(m, r->a[5] + 0x30F0) & 4u)) {
        wr16(m, obj + 0x3E, 0);
        goto rearm;
    }

    int16_t timer = (int16_t)rd16(m, obj + 0x3E);
    if (timer < 0x3C) {
        wr16(m, obj + 0x3E, (uint16_t)(timer + 1));
        if (rd8(m, obj + 0x3F) & 1u)
            goto rearm;
        wr8(m, obj + 0x1F, 0x34);
        wr8(m, obj + 0x1E, 0x34);
        wr16(m, obj + 0x50, 0xFF98);
        wr16(m, obj + 0x52, 0xFFE0);
        r->d[0] = with_word(r->d[0], rd16(m, obj + 0x3E) >> 3);
        wr16(m, obj + 0x50, (uint16_t)(rd16(m, obj + 0x50) - r->d[0]));
        wr8(m, obj + 0x1D, 0xEA);
        wr32(m, obj + 0x10, 0x4F138);
        draw_projected(m, r, 0x13A156);
        goto rearm;
    }
    if (timer == 0x3C) { /* 0x13A15A */
        FnAuto_PlayHooked(m, r, 0x27, 0x13A164, 0x13A16A);
        wr16(m, obj + 0x3E, (uint16_t)(rd16(m, obj + 0x3E) + 1u));
    }

    wr16(m, obj + 0x3A, (uint16_t)(rd16(m, obj + 0x3A) + 0x40u));
    if ((int8_t)rd8(m, obj + 0x3A) >= 3)
        wr8(m, obj + 0x3A, 0);
    wr8(m, obj + 0x1F, 0);
    wr8(m, obj + 0x1E, 0x2B);
    wr16(m, obj + 0x50, 0xFF7B);
    wr16(m, obj + 0x52, 0xFFD8);
    wr8(m, obj + 0x1D, 0xC8);
    r->d[0] = rd8(m, obj + 0x3A);

    /* 0x13A1AA + 0xA2 = 0x13A24C, 0x13A1C2 + 0x7E = 0x13A240. */
    wr32(m, obj + 0x10, rd32(m, 0x13A24Cu + 4u * r->d[0]));
    push_long(m, r, r->d[0]);
    draw_projected(m, r, 0x13A1BC);
    r->d[0] = rd32(m, r->a[7]);
    r->a[7] += 4;
    wr32(m, obj + 0x10, rd32(m, 0x13A240u + 4u * r->d[0]));
    push_long(m, r, r->d[0]);
    draw_projected(m, r, 0x13A1D4);
    r->d[0] = rd32(m, r->a[7]);
    r->a[7] += 4;

    wr16(m, obj + 0x2C, 0);
    wr16(m, obj + 0x2E, 0x20);
    for (;;) { /* 0x13A1E4 */
        wr16(m, obj + 0x14, (uint16_t)(rd16(m, obj + 0x14) - 0x30u));
        wr16(m, obj + 0x2E, (uint16_t)(rd16(m, obj + 0x2E) + 0x30u));
        if ((int16_t)rd16(m, obj + 0x14) <= -0x30)
            break;
        uint16_t index = (uint16_t)(r->d[0] + 1u);
        if ((int16_t)index >= 3)
            index = 0;
        r->d[0] = with_word(r->d[0], index);
        /* 0x13A208 + 0x44 = 0x13A24C. */
        wr32(m, obj + 0x10, rd32(m, 0x13A24Cu + (uint32_t)((int32_t)(int16_t)index * 4)));
        push_long(m, r, r->d[0]);
        draw_sprite(m, r, 0x13A21A);
        r->d[0] = rd32(m, r->a[7]);
        r->a[7] += 4;
    }

    push_long(m, r, 0x13A226); /* 0x0CBDAE is the hazard list append. */
    Coll_AddHazard(m, obj);
    r->a[1] = rd32(m, r->a[5] + 0x906);
    r->a[7] += 4;

rearm:
    wr32(m, obj, 0x13A0EE); /* 0x13A22E -> 0x13A0E6 */
    return 0;
}
