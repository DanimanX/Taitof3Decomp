#ifndef FN_SPRITE_REGS_H
#define FN_SPRITE_REGS_H

#include "fn_call_frames.h"
#include "../video/sprite.h"

/* 0x001282: use the sprite writer for RAM, and track its exposed scratch
 * registers. The grid walk here only tracks registers affected by tiles. */
static inline void call_draw_sprite(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    uint32_t obj = r->a[0], data = rd32(m, obj + 0x10u);
    uint32_t header = rd32(m, data);
    uint8_t mode = rd8(m, obj + 0x1Cu), colour = rd8(m, obj + 0x1Du);
    r->a[2] = data;
    if (header == 0x00010001u) { /* 0x0013CE */
        uint16_t attr = rd16(m, data + 6u) ^ ((uint16_t)mode << 8);
        if (colour)
            attr = (attr & 0xFF00u) | colour;
        r->d[0] = ((uint32_t)(rd16(m, obj + 0x14u) & 0x0FFFu) << 16) | attr;
        r->d[1] = set_word(r->d[1], (uint16_t)mode << 8);
    } else { /* 0x001294 */
        uint16_t cols = (uint16_t)(header >> 16) - 1u, rows = (uint16_t)header - 1u;
        uint16_t pitch = (uint16_t)((cols + 1u) * 4u);
        int32_t step = (mode & 2u) ? -4 : 4;
        uint32_t d2 = 0;
        data += 4u;
        if (mode & 1u) {
            pitch = (uint16_t)-pitch;
            d2 = (uint32_t)((int32_t)(int16_t)rows * (int16_t)pitch);
            data -= (int16_t)d2;
        }
        if (mode & 2u)
            data += (int32_t)(int16_t)cols * 4;
        uint32_t flags = set_word(r->d[4], (uint16_t)mode << 8);
        uint16_t zoom = rd16(m, obj + 0x1Eu);
        uint16_t dy = (uint16_t)((zoom >> 8) - 0x100u);
        uint32_t attr = rd16(m, obj + 0x14u) & 0x0FFFu;
        for (uint32_t y = 0; y <= rows; ++y) {
            r->a[3] = data;
            uint16_t sprite_y = (uint16_t)(rd16(m, obj + 0x18u) << 4);
            flags &= ~0x80003000u;
            for (uint32_t x = 0; x <= cols; ++x) {
                if (rd16(m, data)) {
                    uint16_t tile_attr = rd16(m, data + 2u) ^ (uint16_t)flags;
                    if (colour)
                        tile_attr = (tile_attr & 0xFF00u) | colour;
                    attr = set_word(attr, tile_attr);
                    if (!(flags & 0x1000u)) {
                        flags = (flags | 0x80007000u) & ~0x8000u;
                        attr = sprite_y >> 4;
                    }
                } else {
                    flags &= ~0x3000u;
                }
                data += step;
                sprite_y -= dy;
            }
            d2 = set_word(d2, sprite_y);
            data = r->a[3] + (int16_t)pitch;
            if (!(flags & 0x80000000u))
                flags |= 0x4000u;
            flags |= 0x8000u;
        }
        r->d[0] = 0xFFFFu;
        r->d[1] = ((uint32_t)cols << 16) | 0xFFFFu;
        r->d[2] = d2;
        r->d[3] = attr;
        r->d[4] = flags;
        r->d[5] = ((uint32_t)(uint16_t)((zoom & 0xFFu) - 0x101u) << 16) | dy;
        r->d[6] = set_word(r->d[6], (uint16_t)step);
        r->d[7] = set_word(r->d[7], pitch);
        r->a[2] = data;
    }
    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, r->a[5] - 0x0CA0u);
    r->a[7] += 4u;
}

#endif
