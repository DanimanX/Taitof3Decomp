#include "fn_auto.h"
/* ROM 0x121254: UNVERIFIED (weak capture): animate and draw relative to the shared coordinates, or free when the parent signals. */
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t with_word(uint32_t reg, uint16_t word)
{
    return (reg & 0xFFFF0000u) | word;
}

/* 0x0CBD52: the final frame offset and timer remain in D0.w and D1.w. */
static void animate_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    Obj_Animate(m, obj, r->a[1]);
    r->d[0] = with_word(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = with_word(r->d[1], rd16(m, obj + 0x3Eu));
}

/* 0x001282: use the shared sprite writer and recover the scratch registers
 * from the same sprite definition and row traversal. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t def = rd32(m, obj + 0x10u);
    uint32_t header = rd32(m, def);
    uint16_t x = rd16(m, obj + 0x14u) & 0x0FFFu;
    uint16_t y = rd16(m, obj + 0x18u);
    uint16_t flip = rd16(m, obj + 0x1Cu) & 0xFF00u;
    uint8_t colour = rd8(m, obj + 0x1Du);

    if (header == 0x00010001u) { /* 0x0013CE */
        uint16_t attr = rd16(m, def + 6u) ^ flip;
        if (colour)
            attr = (attr & 0xFF00u) | colour;
        r->d[0] = ((uint32_t)x << 16) | attr;
        r->d[1] = with_word(r->d[1], flip);
        r->a[2] = def;
    } else {
        uint16_t cols = (uint16_t)((header >> 16) - 1u);
        uint16_t rows = (uint16_t)(header - 1u);
        /* 0x12B2 extension 0x2400 indexes the four long pointers at
         * 0x12B8. Their handlers implement the two flip bits below. */
        uint16_t mode = flip >> 8;
        int16_t tile_stride = (mode & 2u) ? -4 : 4;
        int16_t row_stride = (int16_t)((header >> 16) * 4u);
        uint32_t row = def + 4u;
        uint32_t d2 = mode;
        if (mode & 1u) {
            row_stride = (int16_t)-(int32_t)row_stride;
            d2 = (uint32_t)((int32_t)(int16_t)rows * row_stride);
            row -= (uint32_t)(int32_t)(int16_t)d2;
        }
        if (mode & 2u)
            row += (uint32_t)((int32_t)(int16_t)cols * 4);

        uint32_t flags = with_word(r->d[4], flip);
        uint16_t ystep = (uint16_t)(rd8(m, obj + 0x1Eu) - 0x100u);
        uint16_t tile_y;
        uint16_t scratch = x;
        /* 0x001330–0x0013C4: zero tiles break a block. The first tile of
         * each block leaves its Y in D3; subsequent tiles leave the attribute. */
        for (uint32_t j = 0; j <= rows; ++j) {
            uint32_t tile = row;
            tile_y = (uint16_t)(y << 4);
            flags &= ~0x80003000u;
            for (uint32_t i = 0; i <= cols; ++i) {
                if (rd16(m, tile) == 0u) {
                    flags &= ~0x3000u;
                } else {
                    scratch = rd16(m, tile + 2u) ^ (uint16_t)flags;
                    if (colour)
                        scratch = (scratch & 0xFF00u) | colour;
                    if (!(flags & 0x1000u)) {
                        flags = (flags | 0x80007000u) & ~0x8000u;
                        scratch = tile_y >> 4;
                    }
                }
                tile += (uint32_t)(int32_t)tile_stride;
                tile_y = (uint16_t)(tile_y - ystep);
            }
            if (!(flags & 0x80000000u))
                flags |= 0x4000u;
            flags |= 0x8000u;
            r->a[3] = row;
            row += (uint32_t)(int32_t)row_stride;
        }
        r->a[2] = row;
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)cols << 16) | 0xFFFFu;
        r->d[2] = with_word(d2, tile_y);
        r->d[3] = scratch;
        r->d[4] = flags;
        r->d[5] = ((uint32_t)(uint16_t)(rd8(m, obj + 0x1Fu) - 0x101u) << 16) | ystep;
        r->d[6] = with_word(r->d[6], (uint16_t)tile_stride);
        r->d[7] = with_word(r->d[7], (uint16_t)row_stride);
    }
    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, r->a[5] - 0x0CA0u);
    r->a[7] += 4u; /* 0x0013CC or 0x00140E RTS */
}

/* 0x11B09A: place the sprite, cull it, then remove the local offsets. */
static void draw_at_shared_coords(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint16_t x = (uint16_t)(rd16(m, obj + 0x50u) +
                            rd16(m, obj + 0x54u) + rd16(m, a5 + 0x30D2u));
    wr16(m, obj + 0x14u, x);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x52u) +
                            rd16(m, obj + 0x56u) + rd16(m, a5 + 0x30D6u));
    wr16(m, obj + 0x18u, y);

    if ((int16_t)y < 0xE8 && (int16_t)x < 0x140) {
        r->a[1] = rd32(m, obj + 0x10u);
        uint16_t height_test = (uint16_t)((rd16(m, r->a[1]) << 4) + y);
        r->a[1] += 2u;
        if ((int16_t)height_test > 0) {
            uint16_t width_test = (uint16_t)((rd16(m, r->a[1]) << 4) + x);
            r->a[1] += 2u;
            if ((int16_t)width_test > 0) {
                r->a[7] -= 4u;
                wr32(m, r->a[7], 0x11B0F0u); /* 0x11B0EA JSR */
                draw_sprite_regs(m, r);
            }
        }
    }
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t dy = rd16(m, obj + 0x56u);
    r->d[0] = with_word(r->d[0], dy);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dy));
    r->a[7] += 4u; /* 0x11B100 RTS */
}

int Fn_121254(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];
    r->a[1] = rd32(m, obj + 8u);
    if (rd8(m, r->a[1] + 0x59u) != 0u) {
        /* 0x121298: push the argument, return address and 0x0105EC MOVEM. */
        wr32(m, sp - 4u, obj);
        wr32(m, sp - 8u, 0x1212A0u);
        wr32(m, sp - 12u, r->a[1]);
        wr32(m, sp - 16u, obj);
        wr32(m, sp - 20u, r->d[0]);
        Obj_Free(m, obj);
    } else {
        /* 0x121260: bitfield offset 12 selects bit 3 of the second byte. */
        if (rd8(m, a5 + 0x30EFu) & 0x08u) {
            r->d[0] = rd32(m, obj + 0x36u);
            wr32(m, obj + 0x36u, rd32(m, a5 + 0x30D6u));
            /* LEA bases 0x12127C and 0x121286 plus 0x2A and 0x46. */
            r->a[1] = (int32_t)r->d[0] > (int32_t)rd32(m, a5 + 0x30D6u)
                ? 0x1212A6u : 0x1212CCu;
            r->a[7] -= 4u;
            wr32(m, r->a[7], 0x121290u); /* 0x12128A JSR */
            animate_regs(m, r);
            r->a[7] += 4u; /* 0x0CBD84 RTS */
            r->a[7] -= 4u;
            wr32(m, r->a[7], 0x121296u); /* 0x121290 JSR */
            draw_at_shared_coords(m, r);
        }
        wr32(m, obj, 0x121254u); /* 0x12124C */
    }
    r->a[7] += 4u;
    return 0;
}
