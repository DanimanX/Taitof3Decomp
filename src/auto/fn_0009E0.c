#include "fn_auto.h"
/* ROM 0x0009E0: queue a 0xB8-family serial command with two packed numbers. */
#include "fn_call_frames.h"

int Fn_0009E0(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    r->a[6] = r->a[7];
    uint8_t cmd = rd8(m, r->a[6] + 8) | 0xB8u;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | cmd;
    r->a[7] -= 2;
    wr8(m, r->a[7], cmd);
    push_long(m, r, 0x0009F2);
    Fn_000C8A(m, r);
    r->a[7] += 2;

    push_word(m, r, 5);
    push_long(m, r, rd32(m, r->a[6] + 10));
    push_long(m, r, 0x000A00);
    Fn_000C30(m, r);
    r->a[7] += 6;

    r->d[0] = (uint32_t)(int32_t)(int16_t)rd16(m, r->a[6] + 14);
    push_word(m, r, 2);
    push_long(m, r, r->d[0]);
    push_long(m, r, 0x000A12);
    Fn_000C30(m, r);
    r->a[7] += 6;

    r->a[7] -= 2;
    wr8(m, r->a[7], 0xFF);
    push_long(m, r, 0x000A1C);
    Fn_000C8A(m, r);
    r->a[7] += 2;
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8;
    return 0;
}
