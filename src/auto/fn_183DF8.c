#include "fn_auto.h"
/* ROM 0x183DF8: follow a parent and periodically spawn a three-way aimed volley. */
#include "fn_boss_camera.h"

/* ROM 0x0CC2BC: frame-rate-scaled modulo gate, as in Fn_183C32. */
static void volley_gate(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, 0x183E10u);
    uint16_t period = (uint16_t)(0x180u - rd16(m, r->a[5] + 0x87Eu));
    uint32_t scaled = ((uint32_t)period * (uint16_t)r->d[0]) << 8;
    r->d[1] = (scaled >> 16) | (scaled << 16);
    if ((uint16_t)r->d[1] == 0u)
        r->d[1] = 1u;
    uint16_t tick = rd16(m, r->a[5] + 0x878u);
    uint16_t divisor = (uint16_t)r->d[1];
    r->d[0] = ((uint32_t)(tick / divisor) << 16) | (uint16_t)(tick % divisor);
    r->a[7] += 4u;
}

/* ROM 0x17AF9A: stack-argument wrapper for the shared 0x006422 angle lookup. */
static void aim_volley(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t obj = r->a[0];
    push_word(m, r, rd16(m, obj + 0x54u));
    push_word(m, r, rd16(m, obj + 0x50u));
    push_long(m, r, rd32(m, obj + 0x4Cu));
    push_long(m, r, 0x183E4Cu);
    push_long(m, r, r->a[6]);
    r->a[6] = r->a[7];
    push_long(m, r, r->a[1]);
    push_long(m, r, r->d[1]);

    r->d[0] = rd32(m, r->a[6] + 8u);
    if ((int32_t)r->d[0] > 0) {
        r->a[1] = r->d[0];
        uint16_t dx = (uint16_t)(rd16(m, r->a[1] + 0x14u) + 0x18u - rd16(m, r->a[6] + 0xCu));
        uint16_t dy = (uint16_t)(rd16(m, r->a[1] + 0x18u) + 0x10u - rd16(m, r->a[6] + 0xEu));
        r->d[0] = set_word(r->d[0], dx);
        r->d[1] = set_word(r->d[1], dy);
        boss_angle(m, r, 0x17AFCCu);
    }
    r->d[1] = rd32(m, r->a[7]);
    r->a[1] = rd32(m, r->a[7] + 4u);
    r->a[6] = rd32(m, r->a[6]);
    r->a[7] = sp;
}

static void adjust_angle(f3regs_t *r, uint8_t delta)
{
    r->d[1] = (r->d[1] & 0xFFFFFF00u) | (uint8_t)(r->d[1] + delta);
}

int Fn_183DF8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[4] = rd32(m, obj + 8u);
    if (!(rd8(m, r->a[4] + 0x23u) & 4u)) {
        r->d[0] = set_word(r->d[0], 10u);                         /* 0x183E06 */
        volley_gate(m, r);
        if ((uint16_t)r->d[0] == 0u) {
            r->a[4] = rd32(m, obj + 8u);
            uint16_t x = (uint16_t)(rd16(m, r->a[4] + 0x50u) + rd16(m, obj + 0x16u));
            uint16_t y = (uint16_t)(rd16(m, r->a[4] + 0x54u) + rd16(m, obj + 0x1Au));
            r->d[0] = set_word(r->d[0], x);
            r->d[1] = set_word(r->d[1], y);
            wr16(m, obj + 0x50u, x);
            wr16(m, obj + 0x54u, y);
            wr16(m, obj + 0x14u, x);
            wr16(m, obj + 0x18u, y);

            aim_volley(m, r);                                    /* 0x183E46 */
            r->d[1] = (uint32_t)(int32_t)(int16_t)r->d[0];
            adjust_angle(r, (uint8_t)-10);
            call_spawn(m, r, 0x183E6Eu, 0x18417Au, 9u, 2u, r->d[1]);
            adjust_angle(r, 10u);
            call_spawn(m, r, 0x183E8Cu, 0x184184u, 9u, 2u, r->d[1]);
            adjust_angle(r, 10u);
            call_spawn(m, r, 0x183EAAu, 0x18417Au, 9u, 2u, r->d[1]);
        }

        uint16_t timer = rd16(m, obj + 0x64u);                    /* 0x183EAE */
        wr16(m, obj + 0x64u, (uint16_t)(timer - 1u));
        if ((int16_t)timer > 1) {
            wr32(m, obj, 0x183DF8u);                             /* 0x183DF0 */
            r->a[7] += 4u;
            return 0;
        }
    }

    wr16(m, obj + 6u, 1u);                                      /* 0x183EB6 */
    wr32(m, obj, 0x183EC6u);                                     /* 0x183EBE */
    r->a[7] += 4u;
    return 0;
}
