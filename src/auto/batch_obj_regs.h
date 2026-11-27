#ifndef BATCH_OBJ_REGS_H
#define BATCH_OBJ_REGS_H

#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static inline uint32_t batch_set_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

/* ROM 0x0105EC preserves D0/A0/A1. Mirror its caller frame and MOVEM. */
static inline void batch_free_object(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, r->a[0]);
    wr32(m, sp - 8u, ret);
    wr32(m, sp - 12u, r->a[1]);
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 20u, r->d[0]);
    Obj_Free(m, rd32(m, sp - 4u));
    r->d[0] = rd32(m, sp - 20u);
    r->a[0] = rd32(m, sp - 16u);
    r->a[1] = rd32(m, sp - 12u);
}

/* ROM 0x001282. Obj_DrawSprite writes the sprites; track its scratch
 * registers from the source tiles and the block-control instructions. */
static inline void batch_sprite_outputs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t source = rd32(m, obj + 0x10u);
    uint16_t x = rd16(m, obj + 0x14u);
    uint16_t y = rd16(m, obj + 0x18u);
    uint16_t flip = rd16(m, obj + 0x1Cu) & 0xFF00u;
    uint8_t colour = rd8(m, obj + 0x1Du);

    if (rd32(m, source) == 0x00010001u) { /* 0x0013CE */
        uint16_t attr = rd16(m, source + 6u) ^ flip;
        if (colour != 0)
            attr = (attr & 0xFF00u) | colour;
        r->a[2] = source;
        r->d[0] = ((uint32_t)(x & 0x0FFFu) << 16) | attr;
        r->d[1] = batch_set_word(r->d[1], flip);
    } else {
        uint16_t cols = rd16(m, source);
        uint16_t rows = rd16(m, source + 2u);
        unsigned flags = flip >> 8;
        int16_t col_step = (flags & 2u) ? -4 : 4;
        int16_t row_step = (int16_t)(cols * 4u);
        uint32_t tile = source + 4u;
        if (flags & 1u) {
            row_step = (int16_t)-row_step;
            tile -= (int32_t)(int16_t)((int16_t)(rows - 1u) * row_step);
        }
        if (flags & 2u)
            tile += (int32_t)(int16_t)(cols - 1u) * 4;

        uint16_t zoom_step = (uint16_t)(rd8(m, obj + 0x1Eu) - 0x100u);
        uint32_t control = batch_set_word(r->d[4], flip);
        uint32_t last_attr = x & 0x0FFFu;
        uint16_t row_y;
        uint32_t row_start;
        uint16_t row = (uint16_t)(rows - 1u);
        do { /* 0x001330 */
            row_start = tile;
            row_y = (uint16_t)(y << 4);
            control &= ~0x80003000u;
            uint16_t col = (uint16_t)(cols - 1u);
            do {
                if (rd16(m, tile) == 0) {
                    control &= ~0x3000u;
                } else {
                    uint16_t attr = rd16(m, tile + 2u) ^ (uint16_t)control;
                    if (colour != 0)
                        attr = (attr & 0xFF00u) | colour;
                    last_attr = batch_set_word(last_attr, attr);
                    if ((control & 0x1000u) == 0) {
                        control = (control | 0x80007000u) & ~0x8000u;
                        last_attr = row_y >> 4;
                    }
                }
                tile += col_step;
                row_y = (uint16_t)(row_y - zoom_step);
            } while (col-- != 0);
            tile = row_start + row_step;
            if ((control & 0x80000000u) == 0)
                control |= 0x4000u;
            control |= 0x8000u;
        } while (row-- != 0);

        r->a[2] = tile;
        r->a[3] = row_start;
        r->d[0] = 0xFFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0xFFFFu;
        r->d[2] = row_y;
        r->d[3] = last_attr;
        r->d[4] = control;
        r->d[5] = ((uint32_t)(uint16_t)(rd8(m, obj + 0x1Fu) - 0x101u) << 16)
                | zoom_step;
        r->d[6] = batch_set_word(r->d[6], (uint16_t)col_step);
        r->d[7] = batch_set_word(r->d[7], (uint16_t)row_step);
    }
    r->a[1] = rd32(m, r->a[5] - 0x0CA0u);
}

static inline void batch_draw_sprite(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    wr32(m, r->a[7] - 4u, ret);
    Obj_DrawSprite(m, r->a[0]);
    batch_sprite_outputs(m, r);
}

#endif
