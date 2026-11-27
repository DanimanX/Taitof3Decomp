#include "fn_auto.h"
/* ROM 0x12C15C: follows and draws the parent sprite before launching the attached projectile. */
#include "round8_projectile_regs.h"

int Fn_12C15C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    if (rd8(m, a5 - 0x144Bu) & 0x10u) { /* 0x12C15C -> 0x12C2A6 */
        wr8(m, a5 + 0x329Bu, (uint8_t)(rd8(m, a5 + 0x329Bu) - 1u));
        call_free(m, r, 0x12C2B2u);
        r->a[7] += 4u;
        return 0;
    }

    r->a[1] = rd32(m, obj + 8u); /* 0x12C166 */
    wr32(m, obj + 0x14u, rd32(m, r->a[1] + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, r->a[1] + 0x18u));
    wr32(m, obj + 0x10u, rd32(m, r->a[1] + 0x10u));
    wr8(m, obj + 0x1Cu, rd8(m, r->a[1] + 0x1Cu));
    if ((rd8(m, obj + 7u) & 1u) == 0)
        draw_projectile(m, r, 0x12C192u); /* 0x12C18C */

    uint16_t timer = rd16(m, obj + 6u); /* 0x12C192 */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x12C15Cu); /* 0x12C154 */
    } else {
        wr8(m, a5 + 0x329Bu, (uint8_t)(rd8(m, a5 + 0x329Bu) + 1u));
        r->a[1] = rd32(m, obj + 8u);
        uint16_t angle = rd16(m, r->a[1] + 0x20u);
        wr16(m, obj + 0x3Eu, angle);
        angle = (uint16_t)(angle - 0x4000u);
        wr16(m, obj + 0x20u, angle);
        uint16_t turn = (uint16_t)((int16_t)(uint16_t)(0u - angle) >> 6);
        r->d[0] = set_word(r->d[0], turn);
        wr16(m, obj + 0x22u, turn);
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 6u, 0x0800u);
        wr32(m, obj, 0x12C1D0u); /* 0x12C1C8 */
    }

    r->a[7] += 4u;
    return 0;
}
