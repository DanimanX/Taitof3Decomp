#include "fn_auto.h"
/* ROM 0x000A22: queue a 0xC0-family serial command and its terminator. */
#include "fn_call_frames.h"

int Fn_000A22(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[6]);
    r->a[6] = r->a[7];
    uint8_t cmd = rd8(m, r->a[6] + 8) | 0xC0u;
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | cmd;
    r->a[7] -= 2;
    wr8(m, r->a[7], cmd);
    push_long(m, r, 0x000A34);
    Fn_000C8A(m, r);
    r->a[7] += 2;
    r->a[7] -= 2;
    wr8(m, r->a[7], 0xFF);
    push_long(m, r, 0x000A3E);
    Fn_000C8A(m, r);
    r->a[7] += 2;
    r->a[7] = r->a[6];
    r->a[6] = rd32(m, r->a[7]);
    r->a[7] += 8;
    return 0;
}
