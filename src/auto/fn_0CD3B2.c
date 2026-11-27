#include "fn_auto.h"
/* ROM 0x0CD3B2: Select the directional sprite from the A1 table and set its flip byte. */
#include "fn_call_frames.h"

int Fn_0CD3B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t angle = rd16(m, obj + 0x20u);
    angle = (angle & 0xFF00u) | (uint8_t)(angle + 4u); /* 0x0CD3B6 ADDQ.B */
    r->d[1] = (angle >> 6) & 3u; /* 0x0CD3B8 BFEXTU {24:2} */
    uint16_t index = (angle >> 3) & 0xFu;
    r->d[0] = set_word(r->d[0], index);
    wr32(m, obj + 0x10u, rd32(m, r->a[1] + 4u * index)); /* 0x0CD3C2 */
    r->a[1] = 0x0CC10Eu;
    wr8(m, obj + 0x1Cu, rd8(m, r->a[1] + r->d[1])); /* 0x0CD3CE */
    r->a[7] += 4u;
    return 0;
}
