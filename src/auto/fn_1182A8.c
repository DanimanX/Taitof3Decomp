#include "fn_auto.h"
/* ROM 0x1182A8: UNVERIFIED (no capture): Decay the horizontal zoom, clamp the sprite to the viewport, and draw or free it. */
#include "batch_obj_regs.h"

static uint16_t clamp_word(uint16_t value, uint16_t low, uint16_t high)
{
    if ((int16_t)value < (int16_t)low)
        value = low;
    if ((int16_t)value > (int16_t)high)
        value = high;
    return value;
}

int Fn_1182A8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    if ((rd8(m, a5 - 0x144Bu) & 8u) != 0) {
        batch_free_object(m, r, 0x118336u); /* 0x1182AE -> 0x11832E */
        return 0;
    }

    uint16_t zoom = rd16(m, obj + 0x22u);
    uint16_t offset = 0;
    if (zoom != 0) {
        wr8(m, obj + 0x1Eu, (uint8_t)zoom);
        offset = (uint8_t)zoom >> 4;
        wr16(m, obj + 0x22u, (uint16_t)(zoom - 8u));
        if ((int16_t)zoom < 8)
            wr16(m, obj + 0x22u, 0u);
    }

    r->d[4] = batch_set_word(r->d[4], offset); /* 0x1182D0 */
    uint16_t x = rd16(m, obj + 0x52u);
    uint16_t y = (uint16_t)(offset + rd16(m, obj + 0x54u));
    uint16_t x_low = rd16(m, a5 + 0x30C2u);
    uint16_t x_high = (uint16_t)(rd16(m, a5 + 0x30C4u) - 0x20u);
    x = clamp_word(x, x_low, x_high);
    uint16_t y_low = rd16(m, a5 + 0x30C6u);
    uint16_t y_high = (uint16_t)(rd16(m, a5 + 0x30C8u) - 0x20u + offset);
    y = clamp_word(y, y_low, y_high);
    r->d[2] = batch_set_word(r->d[2], y_low);
    r->d[3] = batch_set_word(r->d[3], y_high);
    x = (uint16_t)(x + rd16(m, a5 + 0x30D2u));
    r->d[0] = batch_set_word(r->d[0], x);
    wr16(m, obj + 0x14u, x);
    y = (uint16_t)(y + rd16(m, a5 + 0x30D6u));
    r->d[1] = batch_set_word(r->d[1], y);
    wr16(m, obj + 0x18u, y);
    batch_draw_sprite(m, r, 0x11832Au);
    wr32(m, obj, 0x1182A8u); /* 0x11832A -> 0x1182A0 */
    return 0;
}
