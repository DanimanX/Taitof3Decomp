/* ROM 0x16742C: object step (A0 = object). Sets (+0x44).w = 0x80 and
 * re-arms the object with 0x16743A. */
#include "fn_auto.h"

int Fn_16742C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x44u, 0x0080u);                       /* 0x16742C */
    wr32(m, obj + 0x00u, 0x0016743Au);                   /* 0x167432 */
    return 0;
}
