/* ROM 0x18E708: initialize +0x06 = 0x14 and arm 0x18E718 (A0 = object). */
#include "fn_auto.h"

int Fn_18E708(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x06u, 0x0014u);                       /* 0x18E708 (0x18E70E beq folded) */
    wr32(m, obj, 0x0018E718u);                           /* 0x18E710 */
    return 0;
}
