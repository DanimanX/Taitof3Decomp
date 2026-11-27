#include "fn_auto.h"
/* ROM 0x139FE4: verified: the capture discriminates (an empty body fails it): Update a boss attachment's shared damage and draw five independently phased sprites. */
#include "batch_obj_regs.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x11AE62. The caller supplies D0=100000 and D1=3. */
static void apply_damage(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t hp_addr = r->a[5] + 0x30DAu;
    uint32_t hp = rd32(m, hp_addr);
    if ((int32_t)hp <= 0)
        return;
    r->d[2] = rd32(m, obj + 0x24u);
    if (r->d[2] == 0)
        return;
    wr32(m, obj + 0x24u, 0u);
    r->a[1] = rd32(m, obj + 0x36u);
    uint8_t kind = rd8(m, r->a[1] + 0x22u);
    if (kind != 0 && kind != 4) {
        r->d[2] = (uint32_t)((int32_t)r->d[2] >> 3);
        if (kind != 1 && kind != 5)
            r->d[2] = (uint32_t)((int32_t)r->d[2] >> 1);
    }
    hp += r->d[2];
    wr32(m, hp_addr, hp);
    if ((int32_t)hp <= 0) {
        wr32(m, r->a[7] - 8u, 0x11AEB8u);
        r->a[1] = Obj_0CBDDE(m, obj);
        wr32(m, r->a[1] + 0x48u, rd32(m, r->a[1] + 0x48u) + r->d[0]);
    }
}

static uint16_t edge(const f3mem_t *m, uint32_t obj, uint32_t bound, uint32_t axis)
{
    return (uint16_t)(rd16(m, obj + bound) + rd16(m, obj + axis));
}

static uint16_t midpoint(uint16_t lower1, uint16_t lower2, uint16_t upper1, uint16_t upper2)
{
    uint16_t lo = (int16_t)lower1 >= (int16_t)lower2 ? lower1 : lower2;
    uint16_t hi = (int16_t)upper1 <= (int16_t)upper2 ? upper1 : upper2;
    return (uint16_t)((int16_t)(lo + hi) >> 1);
}

/* ROM 0x11C106..0x11C1D8. Insert the position into the reserved slot,
 * preserving its busy bit and the low twelve bits of the record. */
static void place_spark(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t source = rd32(m, obj + 0x36u);
    r->a[1] = source;
    uint16_t x, y;
    if (rd8(m, source + 0x22u) == 6 || rd8(m, obj + 0x35u) == 2) {
        wr32(m, r->a[7] - 8u, 0x11C190u);
        uint32_t sample = (uint16_t)Rng_NextMem(m);
        uint16_t height = (uint16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u));
        r->d[1] = (sample / height << 16) | (sample % height);
        y = (uint16_t)(r->d[1] + edge(m, obj, 0x30u, 0x18u));
        wr32(m, r->a[7] - 8u, 0x11C1AEu);
        sample = (uint16_t)Rng_NextMem(m);
        uint16_t width = (uint16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu));
        r->d[2] = batch_set_word(r->d[2], width);
        r->d[0] = (sample / width << 16) | (sample % width);
        x = (uint16_t)(r->d[0] + edge(m, obj, 0x2Cu, 0x14u));
    } else {
        x = midpoint(edge(m, obj, 0x2Cu, 0x14u), edge(m, source, 0x2Cu, 0x14u),
                     edge(m, obj, 0x2Eu, 0x14u), edge(m, source, 0x2Eu, 0x14u));
        uint16_t bottom = edge(m, obj, 0x32u, 0x18u);
        uint16_t other_bottom = edge(m, source, 0x32u, 0x18u);
        y = midpoint(edge(m, obj, 0x30u, 0x18u), edge(m, source, 0x30u, 0x18u),
                     bottom, other_bottom);
        r->d[2] = batch_set_word(r->d[2], (int16_t)bottom <= (int16_t)other_bottom ? bottom : other_bottom);
        r->d[3] = batch_set_word(r->d[3], other_bottom);
    }

    x = (uint16_t)(x - rd16(m, r->a[5] + 0x30D2u));
    y = (uint16_t)(y - rd16(m, r->a[5] + 0x30D6u));
    r->d[0] = batch_set_word(r->d[0], x);
    r->d[1] = batch_set_word(r->d[1], y);
    uint32_t slot = r->a[2];
    uint16_t high = rd16(m, slot);
    wr16(m, slot, (uint16_t)((high & 0x801Fu) | ((x & 0x3FFu) << 5)));
    uint16_t next = rd16(m, slot + 1u);
    wr16(m, slot + 1u, (uint16_t)((next & 0xE00Fu) | ((y & 0x1FFu) << 4)));
}

/* ROM 0x11C096. Cooldown is per last object and uses the frame delta word. */
static void queue_hit_spark(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 0)
        goto done;
    r->a[1] = rd32(m, obj + 8u);
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | hit;
    wr8(m, r->a[1] + 0x35u, (uint8_t)(rd8(m, r->a[1] + 0x35u) | hit));
    if ((rd8(m, obj + 0x34u) & 2u) != 0)
        goto done;

    uint16_t frame = rd16(m, a5 + 0x878u);
    uint16_t age = (uint16_t)(frame - rd16(m, a5 + 0x311Cu));
    r->d[0] = batch_set_word(r->d[0], age);
    wr16(m, a5 + 0x311Cu, frame);
    if ((int16_t)age < 2) {
        r->d[0] = rd32(m, a5 + 0x3116u);
        wr32(m, a5 + 0x3116u, obj);
        if (r->d[0] == obj) {
            uint16_t count = (uint16_t)(rd16(m, a5 + 0x311Au) + 1u);
            wr16(m, a5 + 0x311Au, count);
            if ((int16_t)count <= 3)
                goto done;
        }
    }

    wr16(m, a5 + 0x311Au, 0u);
    r->a[2] = 0x40B0F6u;
    for (uint16_t left = 7u;; --left) {
        r->d[0] = batch_set_word(r->d[0], left);
        uint8_t flags = rd8(m, r->a[2]);
        wr8(m, r->a[2], flags | 0x80u);
        if ((flags & 0x80u) == 0) {
            place_spark(m, r);
            break;
        }
        r->a[2] += 4u;
        if (left == 0) {
            r->d[0] = batch_set_word(r->d[0], 0xFFFFu);
            break;
        }
    }
done:
    wr8(m, obj + 0x35u, 0u);
}

/* ROM 0x13A054. MOVEM restores the loop state after each draw. */
static void draw_attachment(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t sp = r->a[7];
    uint16_t counter;
    do {
        wr32(m, sp - 4u, r->a[1]);
        wr32(m, sp - 8u, r->d[2]);
        wr32(m, sp - 12u, r->d[1]);
        wr32(m, sp - 16u, r->d[0]);
        wr16(m, sp - 18u, rd16(m, obj + 0x14u));
        wr16(m, sp - 20u, rd16(m, obj + 0x18u));

        uint32_t dx = (uint32_t)(96 * (int16_t)rd16(m, r->a[1] + 2u * r->d[1]));
        uint32_t dy = (uint32_t)(16 * (int16_t)rd16(m, r->a[1] + 2u * r->d[2]));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (dx >> 16) - 24u));
        uint16_t dy_word = (uint16_t)((dy >> 16) - 20u);
        r->d[3] = (dy << 16) | dy_word;
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + dy_word));
        r->a[7] = sp - 20u;
        batch_draw_sprite(m, r, 0x13A08Eu);

        wr16(m, obj + 0x18u, rd16(m, sp - 20u));
        wr16(m, obj + 0x14u, rd16(m, sp - 18u));
        r->d[0] = rd32(m, sp - 16u);
        r->d[1] = (uint8_t)(rd32(m, sp - 12u) + 0x33u);
        r->d[2] = (uint8_t)(rd32(m, sp - 8u) + 0x20u);
        r->a[1] = rd32(m, sp - 4u);
        counter = (uint16_t)(r->d[0] - 1u);
        r->d[0] = batch_set_word(r->d[0], counter);
        r->a[7] = sp;
    } while (counter != 0xFFFFu);
}

int Fn_139FE4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];
    r->a[1] = rd32(m, obj + 8u);
    if (rd8(m, r->a[1] + 0x59u) != 0) {
        batch_free_object(m, r, 0x13A0BAu);
        return 0;
    }
    if ((int16_t)rd16(m, a5 + 0x32D6u) >= 0x0B) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, a5 + 0x30D2u) + rd16(m, obj + 0x50u)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a5 + 0x30D6u) + rd16(m, obj + 0x52u)));
        r->d[0] = 0x186A0u;
        r->d[1] = 3u;
        wr32(m, sp - 4u, 0x13A020u);
        apply_damage(m, r);
        wr32(m, sp - 4u, 0x13A026u);
        queue_hit_spark(m, r);
        wr32(m, sp - 4u, 0x13A02Cu);
        wr32(m, sp - 8u, 0x11B096u);
        Obj_11B06E(m, obj); /* A1 from the collision append is replaced at 0x13A02C. */

        r->a[1] = rd32(m, obj + 8u);
        wr8(m, obj + 0x1Du, rd8(m, r->a[1] + 0x1Du));
        wr32(m, obj + 0x10u, 0x03BC1Cu);
        r->d[0] = batch_set_word(r->d[0], 4u);
        r->d[1] = rd8(m, obj + 0x20u);
        r->d[2] = rd8(m, obj + 0x21u);
        r->a[1] = 0x027E40u;
        draw_attachment(m, r);
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + 1u));
        wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + 2u));
    }
    wr32(m, obj, 0x139FE4u); /* 0x13A0AE -> 0x139FDC */
    return 0;
}
