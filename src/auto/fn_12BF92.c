#include "fn_auto.h"
/* ROM 0x12BF92: Fly a boss projectile relative to its parent and draw it, retire it offscreen, or switch to the follow-up animation. */
#include "batch_obj_regs.h"

static int outside_bounds(const f3mem_t *m, uint32_t bounds, uint16_t value)
{
    int16_t coordinate = (int16_t)value;
    return coordinate < (int16_t)rd16(m, bounds) ||
           coordinate > (int16_t)rd16(m, bounds + 2u);
}

/* ROM 0x0CD3D6, reached by JMP with D0.w = 1. The table at 0x0CD444
 * points at the animation records starting with an optional sound word. */
static void start_followup(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, 0x0CD444u + 4u * (uint16_t)r->d[0]);
    uint16_t cmd = rd16(m, r->a[1]);
    r->a[1] += 2u;
    r->d[0] = batch_set_word(r->d[0], cmd);
    if (cmd != 0)
        FnAuto_PlayHooked(m, r, cmd, 0x0CD3ECu, 0x0CD3F2u);

    wr32(m, obj + 0x0Cu, r->a[1]); /* 0x0CD3F6 */
    wr16(m, obj + 0x3Au, 0);
    wr16(m, obj + 0x3Eu, 0);
    wr16(m, obj + 0x1Eu, 0);
    wr16(m, obj + 0x1Cu, 0);
    wr32(m, obj, 0x0CD412u);
}

/* ROM 0x12C07A through 0x12C0A0. Snap to the final position first. */
static void land_projectile(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, obj + 8u);
    r->d[1] = batch_set_word(r->d[1], (uint16_t)(rd16(m, r->a[1] + 0x14u) + rd16(m, obj + 0x44u)));
    wr16(m, obj + 0x14u, (uint16_t)r->d[1]);
    r->d[1] = batch_set_word(r->d[1], (uint16_t)(rd16(m, r->a[1] + 0x18u) + rd16(m, obj + 0x48u)));
    wr16(m, obj + 0x18u, (uint16_t)r->d[1]);
    wr16(m, obj + 0x42u, 0xAu);
    r->d[0] = batch_set_word(r->d[0], 1);
    start_followup(m, r);
}

/* ROM 0x0CBDAE. Append the object to the hazard collision list. */
static void append_hazard(const f3mem_t *m, f3regs_t *r)
{
    wr32(m, r->a[7] - 4u, 0x12BFF2u);
    uint32_t cursor = r->a[5] + 0x906u;
    r->a[1] = rd32(m, cursor);
    wr32(m, r->a[1], r->a[0]);
    r->a[1] += 4u;
    wr32(m, cursor, r->a[1]);
}

int Fn_12BF92(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];
    r->a[1] = rd32(m, obj + 8u);
    wr8(m, r->a[1] + 0x22u, 0xFFu); /* 0x12BF96 */
    if ((rd8(m, globals - 0x144Bu) & 0x10u) != 0 ||
        (int16_t)rd16(m, globals + 0x30DAu) < 0x1F4)
        goto land;

    /* 0x12BFAE through 0x12BFD0 integrate the relative offsets. */
    uint16_t heading = rd16(m, obj + 0x20u);
    r->d[0] = batch_set_word(r->d[0], heading);
    r->a[1] = 0x027E40u;
    uint32_t trig = r->a[1] + (uint32_t)((int32_t)(int16_t)heading * 2);
    r->d[1] = (uint32_t)((int16_t)rd16(m, obj + 0x42u) * (int16_t)rd16(m, trig + 0x80u));
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + r->d[1]);
    r->d[1] = (uint32_t)((int16_t)rd16(m, obj + 0x42u) * (int16_t)rd16(m, trig));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + r->d[1]);

    wr8(m, obj + 0x1Du, rd8(m, obj + 0x1Du) ^ 0x2Eu); /* 0x12BFD4 */
    heading = rd16(m, obj + 0x20u);
    uint16_t sprite = (uint16_t)((heading & 0xFF00u) | (uint8_t)(heading + 4u)) >> 3;
    r->d[0] = batch_set_word(r->d[0], sprite);
    /* 0x12BFE2 indexed PC EA. Extension at 0x12BFE4 + 0xC2; D0.w * 4. */
    wr32(m, obj + 0x10u, rd32(m, 0x12C0A6u + 4u * sprite));
    append_hazard(m, r);
    if ((rd8(m, globals + 0x1D58u) & 1u) != 0)
        goto land;

    r->d[0] = (r->d[0] & 0xFFFFFF00u) | 0xFFu;
    r->a[1] = rd32(m, obj + 8u);
    uint16_t x = (uint16_t)(rd16(m, r->a[1] + 0x14u) + rd16(m, obj + 0x44u));
    r->d[1] = batch_set_word(r->d[1], x);
    if (outside_bounds(m, 0x0CC112u, x))
        goto retire;
    wr16(m, obj + 0x14u, x);

    uint16_t y = (uint16_t)(rd16(m, r->a[1] + 0x18u) + rd16(m, obj + 0x48u));
    r->d[1] = batch_set_word(r->d[1], y);
    if (outside_bounds(m, 0x0CC116u, y))
        goto retire;
    wr16(m, obj + 0x18u, y);

    batch_draw_sprite(m, r, 0x12C038u);
    r->d[0] &= 0xFFFFFF00u;          /* 0x12C038 SF forces the re-arm branch. */
    wr32(m, obj, 0x12BF92u);        /* 0x12BF8A */
    r->a[7] += 4u;
    return 0;

retire:
    batch_free_object(m, r, 0x12C074u); /* 0x12C06C, via BRA at 0x12C040 */
    r->a[7] += 4u;
    return 0;

land:
    land_projectile(m, r);
    r->a[7] += 4u;
    return 0;
}
