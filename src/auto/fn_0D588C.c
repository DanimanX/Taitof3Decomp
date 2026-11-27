#include "fn_auto.h"
/* ROM 0x0D588C: Select a sprite from the object angle and toggle its flip bits. */
#include "fn_call_frames.h"

int Fn_0D588C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t angle = (uint16_t)(rd16(m, obj + 0x20u) + 0x800u);
    r->a[1] = 0x0D56A6u; /* 0x0D588C: 0x0D588E - 0x1E8 */
    r->d[1] = angle >> 14; /* 0x0D5898 BFEXTU {16:2} */
    uint16_t index = (angle >> 12) & 7u;
    wr32(m, obj + 0x10u, rd32(m, r->a[1] + 4u * index)); /* 0x0D58A4 */
    r->a[1] = 0x0CC10Eu;
    uint8_t flip = rd8(m, r->a[1] + r->d[1]) ^ 3u;
    r->d[0] = set_word(r->d[0], flip); /* The index fits in D0.b. */
    wr8(m, obj + 0x1Cu, rd8(m, obj + 0x1Cu) ^ flip); /* 0x0D58B8 */
    r->a[7] += 4u;
    return 0;
}
