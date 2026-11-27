#include "fn_auto.h"
/* ROM 0x132752: UNVERIFIED (weak capture): Position and draw the ribbon during the scroll window, then launch its follow-up objects. */
#include "fn_sprite_regs.h"

/* 0x1328B8: draw a vertically flipped, rescaled reflection. */
static void draw_reflection(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t obj = r->a[0];
    push_long(m, r, ret);
    push_word(m, r, rd16(m, obj + 0x18u));
    push_word(m, r, rd16(m, obj + 0x1Eu));

    int16_t zoom = (int16_t)rd8(m, obj + 0x1Eu) - 0x100;
    int32_t shift = ((int32_t)zoom * 3) >> 4;
    r->d[0] = (uint32_t)shift;
    r->d[1] = set_word(r->d[1], (zoom >> 3) + 0x100);
    wr16(m, obj + 0x18u, rd16(m, obj + 0x18u) - (uint16_t)shift);
    wr8(m, obj + 0x1Eu, (uint8_t)r->d[1]);
    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x1Cu) | 2u);
    call_draw_sprite(m, r, 0x1328ECu);
    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x1Cu) & ~2u);
    wr16(m, obj + 0x1Eu, rd16(m, r->a[7]));
    wr16(m, obj + 0x18u, rd16(m, r->a[7] + 2u));
    r->a[7] += 8u;
}

/* 0x1327FE: draw the head and up to sixteen trailing segments. */
static void draw_ribbon(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    push_long(m, r, rd32(m, obj + 0x14u));
    push_long(m, r, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x3Au, rd16(m, obj + 0x3Au) + 0x80u);
    uint8_t frame = rd8(m, obj + 0x3Au) & 3u;
    wr8(m, obj + 0x3Au, frame);
    wr16(m, obj + 0x1Eu, 0);
    uint8_t zoom = rd8(m, obj + 0x30u);
    wr8(m, obj + 0x1Eu, zoom);
    wr16(m, obj + 0x18u, rd16(m, obj + 0x18u) + (zoom >> 4) * 3u);
    r->a[1] = 0x13282Cu + 0xD0u;
    r->d[0] = frame;
    wr32(m, obj + 0x10u, rd32(m, r->a[1] + 4u * frame));
    call_draw_sprite(m, r, 0x132840u);
    draw_reflection(m, r, 0x132844u);

    uint8_t width = rd8(m, obj + 0x2Cu);
    wr8(m, obj + 0x1Fu, width);
    r->d[0] = (uint32_t)(((int32_t)width - 0x100) * 0x2000) + 0x8000u;
    r->a[1] = 0x132860u + 0xACu;
    r->d[1] = set_word(r->d[1], frame);
    r->d[2] = set_word(r->d[2], 15u);
    do { /* 0x13286C */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + r->d[0]);
        wr32(m, obj + 0x10u, rd32(m, r->a[1] + 4u * (uint16_t)r->d[1]));
        push_long(m, r, r->a[1]);
        for (unsigned i = 3; i > 0;)
            push_long(m, r, r->d[--i]);
        call_draw_sprite(m, r, 0x132880u);
        draw_reflection(m, r, 0x132884u);
        for (unsigned i = 0; i < 3; ++i)
            r->d[i] = rd32(m, r->a[7] + 4u * i);
        r->a[1] = rd32(m, r->a[7] + 12u);
        r->a[7] += 16u;

        uint16_t next_zoom = rd8(m, obj + 0x1Eu) + 0x10u;
        wr8(m, obj + 0x1Eu, (uint8_t)next_zoom);
        if (next_zoom > 0xFFu) /* 0x13288E BCS */
            break;
        wr16(m, obj + 0x18u, rd16(m, obj + 0x18u) + 3u);
        r->d[1] = set_word(r->d[1], (r->d[1] + 1u) & 3u);
        r->d[2] = set_word(r->d[2], (uint16_t)(r->d[2] - 1u));
    } while ((uint16_t)r->d[2] != 0xFFFFu);

    uint16_t tail = rd16(m, obj + 0x2Cu);
    wr16(m, obj + 0x2Cu, tail - 0x200u);
    if (tail <= 0x200u) /* 0x1328A6 BHI skips the clear */
        wr16(m, obj + 0x2Cu, 0);
    wr32(m, obj + 0x18u, rd32(m, r->a[7]));
    wr32(m, obj + 0x14u, rd32(m, r->a[7] + 4u));
    r->a[7] += 8u;
}

int Fn_132752(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0], globals = r->a[5];
    int32_t y = 0xBE - (int16_t)rd16(m, globals + 0x30D6u);
    r->d[0] = set_word(r->d[0], (uint16_t)y);
    if (y >= 0) { /* 0x13275A BLT uses the signed SUB.W flags. */
        uint16_t index = (uint16_t)y >> 3;
        r->d[0] = set_word(r->d[0], index);
        if (index <= 0x14u) {
            wr32(m, obj + 0x18u, 0x008E0000u);
            uint16_t target = rd16(m, 0x132772u + 0x1AAu + 2u * index);
            target += rd16(m, globals + 0x30D2u);
            uint8_t state = rd8(m, obj + 0x22u);
            wr8(m, obj + 0x22u, state | 1u);
            if (!(state & 1u))
                wr16(m, obj + 0x14u, target);
            r->d[1] = (uint32_t)((int32_t)(int16_t)(target - rd16(m, obj + 0x14u)) * 0x2000);
            wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + r->d[1]);
            push_long(m, r, 0x13279Cu);
            draw_ribbon(m, r);
            r->a[7] += 4u;
            uint16_t fade = rd16(m, obj + 0x30u);
            wr16(m, obj + 0x30u, fade - 0x2000u);
            if (fade <= 0x2000u)
                wr16(m, obj + 0x30u, 0);
        }
    }

    if (rd8(m, globals + 0x30F3u)) { /* 0x1327AE -> 0x13274A */
        wr32(m, obj, 0x132752u);
    } else {
        call_spawn(m, r, 0x1327C8u, 0x16373Cu, 5u, 2u, 0u);
        wr32(m, obj, 0x1327D4u);
    }
    r->a[7] += 4u;
    return 0;
}
