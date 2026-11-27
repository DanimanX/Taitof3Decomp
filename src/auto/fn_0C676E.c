#include "fn_auto.h"
/* ROM 0x0C676E: UNVERIFIED (weak capture): draw debug hitboxes for the collision lists and active players. */
#include "fn_call_frames.h"

/* 0x0C6836: append one corner sprite and publish the new write cursor. */
static void draw_corner(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    uint32_t cursor = r->a[5] - 0xCA0u;
    uint32_t sprite = rd32(m, cursor);
    wr32(m, sprite, r->d[6]);
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] & r->d[4]));
    r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] & r->d[4]));
    wr16(m, sprite + 4u, (uint16_t)r->d[0]);
    wr16(m, sprite + 6u, (uint16_t)r->d[1]);
    wr32(m, sprite + 8u, r->d[5]);
    wr16(m, sprite + 0xCu, 0u);
    r->a[1] = sprite + 0x10u;
    wr32(m, cursor, r->a[1]);
    r->a[7] += 4u;
}

/* 0x0C67EA: walk the four corners, flipping the sprite at each edge. */
static void draw_hitbox(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    push_long(m, r, ret);
    uint32_t obj = r->a[0];
    uint16_t right = rd16(m, obj + 0x2Cu);
    uint16_t bottom = rd16(m, obj + 0x30u);
    r->d[0] = set_word(r->d[0], (uint16_t)(right + rd16(m, obj + 0x14u)));
    r->d[1] = set_word(r->d[1], (uint16_t)(bottom + rd16(m, obj + 0x18u)));
    r->d[2] = set_word(r->d[2], (uint16_t)(right - rd16(m, obj + 0x2Eu) + 15u));
    r->d[3] = set_word(r->d[3], (uint16_t)(bottom - rd16(m, obj + 0x32u) + 15u));
    r->d[5] &= ~(1u << 25);
    draw_corner(m, r, 0x0C6816u);

    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] - r->d[2]));
    r->d[5] |= 1u << 24;
    draw_corner(m, r, 0x0C6820u);

    r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] - r->d[3]));
    r->d[5] |= 1u << 25;
    draw_corner(m, r, 0x0C682Au);

    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] + r->d[2]));
    r->d[5] &= ~(1u << 24);
    draw_corner(m, r, 0x0C6834u);
    r->a[7] += 4u;
}

int Fn_0C676E(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned i = 3; i > 0;)
        push_long(m, r, r->a[--i]);
    for (unsigned i = 8; i > 0;)
        push_long(m, r, r->d[--i]);

    uint32_t a5 = r->a[5];
    if (rd8(m, a5 + 0x917u) != 0u) {
        r->d[6] = rd32(m, 0x05A4F4u) & 0xFFFF0000u;
        r->d[5] = rd32(m, 0x05A4F6u) & 0xFFFF0000u;
        r->d[4] = set_word(r->d[4], 0x0FFFu);

        r->a[2] = 0x41ACE0u;                         /* 0x0C6792 */
        r->d[7] = rd32(m, a5 + 0x906u);
        while (r->a[2] != r->d[7]) {
            r->a[0] = rd32(m, r->a[2]);
            r->a[2] += 4u;
            draw_hitbox(m, r, 0x0C67A8u);
        }

        r->a[2] = 0x41ABE0u;                         /* 0x0C67AA */
        r->d[7] = rd32(m, a5 + 0x902u);
        while (r->a[2] != r->d[7]) {
            r->a[0] = rd32(m, r->a[2]);
            r->a[2] += 4u;
            draw_hitbox(m, r, 0x0C67C0u);
        }

        r->a[0] = 0x41B190u;                         /* 0x0C67C2 */
        r->d[7] = set_word(r->d[7], 1u);
        do {
            if (rd8(m, r->a[0] + 0x34u) == 0u)
                draw_hitbox(m, r, 0x0C67D8u);
            r->a[0] += 0x8Au;
            r->d[7] = set_word(r->d[7], (uint16_t)(r->d[7] - 1u));
        } while ((uint16_t)r->d[7] != 0xFFFFu);
    }

    wr8(m, a5 + 0x917u, 0u);                         /* 0x0C67E0 */
    for (unsigned i = 0; i < 8; ++i, r->a[7] += 4u)
        r->d[i] = rd32(m, r->a[7]);
    for (unsigned i = 0; i < 3; ++i, r->a[7] += 4u)
        r->a[i] = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return 0;
}
