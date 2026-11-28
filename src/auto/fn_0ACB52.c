#include "fn_auto.h"

/* ROM 0x0ACB52: (recipe capture fnk_, work/zcap/recipes15.tsv): clear a sized rectangle in playfield RAM. */
#include "fn_call_frames.h"
#include "../objects/obj_common.h"

int Fn_0ACB52(const f3mem_t *m, f3regs_t *r)
{
    uint32_t descriptor = r->a[0];
    uint32_t sp = r->a[7];

    /* 0x0ACB52 MOVEM.L A1/A0/D7-D0,-(SP), followed by the two arguments. */
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    for (unsigned i = 8; i > 0;)
        push_long(m, r, r->d[--i]);
    push_long(m, r, 0x610084u);
    push_long(m, r, descriptor);
    push_long(m, r, 0x0ACB64u);

    /* 0x0054AC LINK.W A6,#0 and 0x0054B0 MOVEM.L A1/A0/D7-D1,-(SP). */
    push_long(m, r, r->a[6]);
    push_long(m, r, r->a[1]);
    push_long(m, r, r->a[0]);
    for (unsigned i = 8; i > 1;)
        push_long(m, r, r->d[--i]);
    Video_ClearRect(m, descriptor, 0x610084u);

    /* 0x0054E0..0x0ACB6C restore both frames and pop the caller's PC. */
    r->a[7] = sp + 4u;
    return 0;
}
