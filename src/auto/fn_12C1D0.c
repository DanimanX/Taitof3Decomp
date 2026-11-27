#include "fn_auto.h"
/* ROM 0x12C1D0: steers and animates the launched projectile, or selects its exit state. */
#include "round8_projectile_regs.h"

/* ROM 0x0CD3D6 with D0.W = 1 from 0x12C2C6. */
static void begin_exit_animation(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, 0x0CD444u + 4u);
    uint16_t sound = rd16(m, r->a[1]);
    r->a[1] += 2u;
    r->d[0] = set_word(r->d[0], sound);
    if (sound != 0)
        call_hooked_sound(m, r, sound, 0x0CD3ECu, 0x0CD3F2u);
    wr32(m, obj + 0xCu, r->a[1]);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj, 0x0CD412u);
}

int Fn_12C1D0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint32_t active_count = a5 + 0x329Bu;

    if ((rd8(m, a5 - 0x144Bu) & 0x10u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        wr8(m, active_count, (uint8_t)(rd8(m, active_count) - 1u)); /* 0x12C2B8 */
        wr8(m, obj + 0x21u, rd8(m, obj + 0x20u));
        wr8(m, obj + 0x20u, 0u);
        r->d[0] = set_word(r->d[0], 1u);
        begin_exit_animation(m, r);
        r->a[7] += 4u;
        return 0;
    }

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if ((rd8(m, a5 + 0x1D58u) & 2u) && x >= -10 && x <= 0x11C) {
        wr8(m, active_count, (uint8_t)(rd8(m, active_count) - 1u)); /* 0x12C2D0 */
        explode_projectile(m, r);
        r->a[7] += 4u;
        return 0;
    }

    wr8(m, obj + 0x43u, rd8(m, obj + 6u)); /* 0x12C200 */
    r->d[0] = set_word(r->d[0], rd16(m, obj + 0x20u) >> 8);
    push_long(m, r, 0x12C212u);
    move_by_angle_regs(m, r);
    r->a[7] += 4u;
    uint16_t turn = rd16(m, obj + 0x22u);
    r->d[0] = set_word(r->d[0], turn);
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - turn));
    push_long(m, r, 0x12C220u);
    register_projectile(m, r);
    r->a[7] += 4u;

    uint8_t roll = rd8(m, obj + 0x3Eu); /* 0x12C220 */
    wr8(m, obj + 0x3Eu, (uint8_t)(roll + 0x10u));
    wr8(m, obj + 0x1Cu, (roll & 0x80u) ? 0xFFu : 0u);
    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x1Cu) & 3u);
    r->d[0] = (roll >> 3) & 0xFu;
    r->a[1] = rd32(m, obj + 0xCu);
    r->a[2] = rd32(m, r->a[1]);
    r->a[1] += 4u + 8u * r->d[0]; /* 0x12C246 */
    r->a[2] += 4u * r->d[0]; /* 0x12C24A */
    const uint8_t offsets[] = { 0x2Cu, 0x30u, 0x2Eu, 0x32u };
    for (unsigned i = 0; i < sizeof offsets; ++i) {
        int8_t bound = (int8_t)rd8(m, r->a[2]++);
        r->d[0] = (uint16_t)(int16_t)bound;
        wr16(m, obj + offsets[i], (uint16_t)r->d[0]);
    }
    wr32(m, obj + 0x10u, rd32(m, r->a[1]));
    r->a[1] += 4u;
    uint8_t flip = rd8(m, r->a[1]++);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | flip;
    wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ flip));

    draw_projectile(m, r, 0x12C27Eu);
    x = (int16_t)rd16(m, obj + 0x14u);
    r->d[0] = set_word(r->d[0], (uint16_t)x);
    if (outside_word_bounds(m, 0x12C2DAu, x))
        goto free_projectile;
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    r->d[0] = set_word(r->d[0], (uint16_t)y);
    if (outside_word_bounds(m, 0x12C2DEu, y))
        goto free_projectile;

    wr16(m, obj + 6u, (uint16_t)(rd16(m, obj + 6u) + 8u));
    wr32(m, obj, 0x12C1D0u); /* 0x12C1C8 */
    r->a[7] += 4u;
    return 0;

free_projectile: /* 0x12C2A6 */
    wr8(m, active_count, (uint8_t)(rd8(m, active_count) - 1u));
    call_free(m, r, 0x12C2B2u);
    r->a[7] += 4u;
    return 0;
}
