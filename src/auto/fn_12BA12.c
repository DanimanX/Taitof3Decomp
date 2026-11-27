#include "fn_auto.h"
/* ROM 0x12BA12: moves the ricocheting projectile and handles contact, drawing, and retirement. */
#include "round8_projectile_regs.h"

/* ROM 0x12BB2C: find the first strictly overlapping box in the link list. */
static int find_contact(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->d[0] &= 0xFFFFFF00u;
    r->a[2] = rd32(m, r->a[5] + 0x328Eu);

    uint16_t x = rd16(m, obj + 0x14u);
    uint16_t y = rd16(m, obj + 0x18u);
    r->d[1] = set_word(r->d[1], (uint16_t)(x + rd16(m, obj + 0x2Cu)));
    r->d[2] = set_word(r->d[2], (uint16_t)(x + rd16(m, obj + 0x2Eu)));
    r->d[3] = set_word(r->d[3], (uint16_t)(y + rd16(m, obj + 0x30u)));
    r->d[4] = set_word(r->d[4], (uint16_t)(y + rd16(m, obj + 0x32u)));

    while ((int32_t)r->a[2] > 0) { /* 0x12BB52 */
        r->a[1] = r->a[2];
        r->a[2] = rd32(m, r->a[2] + 0x48u);
        x = rd16(m, r->a[1] + 0x14u);
        r->d[6] = set_word(r->d[6], (uint16_t)(x + rd16(m, r->a[1] + 0x2Eu)));
        if ((int16_t)r->d[6] <= (int16_t)r->d[1])
            continue;
        r->d[6] = set_word(r->d[6], (uint16_t)(x + rd16(m, r->a[1] + 0x2Cu)));
        if ((int16_t)r->d[6] >= (int16_t)r->d[2])
            continue;
        y = rd16(m, r->a[1] + 0x18u);
        r->d[6] = set_word(r->d[6], (uint16_t)(y + rd16(m, r->a[1] + 0x32u)));
        if ((int16_t)r->d[6] <= (int16_t)r->d[3])
            continue;
        r->d[6] = set_word(r->d[6], (uint16_t)(y + rd16(m, r->a[1] + 0x30u)));
        if ((int16_t)r->d[6] >= (int16_t)r->d[4])
            continue;
        r->d[0] |= 0xFFu; /* 0x12BB92 */
        return 1;
    }
    return 0;
}

int Fn_12BA12(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) /* 0x12BA12 */
        goto free_projectile;

    register_projectile(m, r); /* 0x12BA1A */
    uint8_t cooldown = rd8(m, obj + 6u);
    if (cooldown != 0) {
        wr8(m, obj + 6u, (uint8_t)(cooldown - 1u));
    } else {
        push_long(m, r, 0x12BA38u);
        int hit = find_contact(m, r);
        r->a[7] += 4u;
        if (hit) { /* 0x12BA3E */
            wr8(m, obj + 6u, 8u);
            uint16_t heading = rd16(m, obj + 0x20u);
            uint8_t surface = rd8(m, r->a[1] + 0x20u);
            heading = (heading & 0xFF00u) | (uint8_t)(2u * surface - (uint8_t)heading);
            r->d[0] = set_word(r->d[0], heading);
            wr16(m, obj + 0x20u, heading);
            call_hooked_sound(m, r, 0x0Fu, 0x12BA60u, 0x12BA66u);
        }
    }

    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x20u)); /* 0x12BA6A */
    push_long(m, r, 0x12BA74u);
    move_by_angle_regs(m, r);
    r->a[7] += 4u;

    uint16_t heading = ((uint16_t)r->d[0] & 0xFF00u) | (uint8_t)(r->d[0] + 4u);
    r->d[1] = (heading >> 6) & 3u; /* 0x12BA76: BFEXTU {24:2} */
    r->d[0] = set_word(r->d[0], (heading >> 3) & 0xFu);
    wr32(m, obj + 0x10u, rd32(m, 0x12BAECu + 4u * (uint16_t)r->d[0]));
    r->a[1] = 0x0CC10Eu;
    wr8(m, obj + 0x1Cu, rd8(m, r->a[1] + r->d[1]));

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if ((rd8(m, r->a[5] + 0x1D58u) & 2u) && x >= -10 && x <= 0x11C) {
        explode_projectile(m, r); /* 0x12BAAE */
        r->a[7] += 4u;
        return 0;
    }

    draw_projectile(m, r, 0x12BABAu);
    x = (int16_t)rd16(m, obj + 0x14u);
    r->d[1] = set_word(r->d[1], (uint16_t)x);
    if (outside_word_bounds(m, 0x0CC112u, x))
        goto free_projectile;
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    r->d[1] = set_word(r->d[1], (uint16_t)y);
    if (outside_word_bounds(m, 0x0CC116u, y))
        goto free_projectile;

    wr32(m, obj, 0x12BA12u); /* 0x12BA0A */
    r->a[7] += 4u;
    return 0;

free_projectile: /* 0x12BADE */
    call_free(m, r, 0x12BAE6u);
    r->a[7] += 4u;
    return 0;
}
