/* ROM 0x19B646: reset an object's extended runtime fields via Obj_17A7FA
 * (0x17A7FA), set the word at +0x40 to 0x0010 and re-arm the tick 0x19B65A. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_19B646(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                              /* 0x19B646 -> 0x17A7FA */
    wr16(m, obj + 0x40u, 0x0010u);                   /* 0x19B64C */
    wr32(m, obj, 0x0019B65Au);                       /* 0x19B652 */
    return 0;
}
