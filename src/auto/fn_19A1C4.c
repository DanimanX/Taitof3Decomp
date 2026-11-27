/* ROM 0x19A1C4: object reset for a script-driven enemy. Clears the extended
 * runtime fields (0x17A7FA), stores the script pointer 0x00045B8C at +0x10,
 * sets +0x16 = 0x000A and +0x1A = -0x10, then re-arms with state 0x19A1E6. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_19A1C4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                    /* 0x19A1C4 jsr 0x17A7FA */
    wr32(m, obj + 0x10u, 0x00045B8Cu);     /* 0x19A1CA */
    wr16(m, obj + 0x16u, 0x000Au);         /* 0x19A1D2 */
    wr16(m, obj + 0x1Au, 0xFFF0u);         /* 0x19A1D8 */
    wr32(m, obj + 0x00u, 0x0019A1E6u);     /* 0x19A1DE -> 0x19A1E6 rts */
    return 0;
}
