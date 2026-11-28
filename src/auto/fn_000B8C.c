/* ROM 0x000B8C: queue serial command 0xE5 and its 0xFF terminator through
 * 0x000C8A (the sound ring producer). */
#include "fn_auto.h"
#include "fn_call_frames.h"

int Fn_000B8C(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 2u;                                /* 0x000B8C move.b #-0x1B,-(SP) */
    wr8(m, r->a[7], 0xE5u);
    push_long(m, r, 0x000B94u);                   /* 0x000B90 bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                /* 0x000B94 addq.l #2,SP */
    r->a[7] -= 2u;                                /* 0x000B96 move.b #-0x1,-(SP) */
    wr8(m, r->a[7], 0xFFu);
    push_long(m, r, 0x000B9Eu);                   /* 0x000B9A bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                /* 0x000B9E addq.l #2,SP */
    return 0;
}
