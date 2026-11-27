/* ROM 0x19EFAA: object step (A0 = object). Sets (+0x06).w = 0x96 and
 * re-arms the object with 0x19EFB8. */
#include "fn_auto.h"

int Fn_19EFAA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x06u, 0x0096u);                       /* 0x19EFAA */
    wr32(m, obj + 0x00u, 0x0019EFB8u);                   /* 0x19EFB0 */
    return 0;
}
