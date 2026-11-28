/* ROM 0x000B76: queue serial command 0xE4 and its 0xFF terminator through
 * 0x000C8A (the sound ring producer). */
#include "fn_auto.h"
#include "fn_call_frames.h"

int Fn_000B76(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 2u;                                /* 0x000B76 move.b #-0x1C,-(SP) */
    wr8(m, r->a[7], 0xE4u);
    push_long(m, r, 0x000B7Eu);                   /* 0x000B7A bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                /* 0x000B7E addq.l #2,SP */
    r->a[7] -= 2u;                                /* 0x000B80 move.b #-0x1,-(SP) */
    wr8(m, r->a[7], 0xFFu);
    push_long(m, r, 0x000B88u);                   /* 0x000B84 bsr 0x000C8A */
    Fn_000C8A(m, r);
    r->a[7] += 2u;                                /* 0x000B88 addq.l #2,SP */
    return 0;
}
