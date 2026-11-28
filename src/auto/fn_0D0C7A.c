/* ROM 0x0D0C7A: register-level wrapper of sub_0D0C7A (src/enemy/obj_CC11C.c)
 * for its capture.
 * `clr.w d0` precedes the table read, so the whole low word comes from table
 * 0x0D0CC2[(A5+0x87F) >> 3] and the entry's upper half survives untouched. */
#include "fn_auto.h"
#include "fn_helpers.h"

int Fn_0D0C7A(const f3mem_t *m, f3regs_t *r)
{
    r->d[0] = (r->d[0] & 0xFFFF0000u) | sub_0D0C7A(m);  /* 0x0D0C7A..0x0D0C8A */
    return 0;
}
