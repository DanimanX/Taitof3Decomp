/* ROM 0x19D1DA: object state init. Clears the word at +0x44, sets the word at
 * +0x46 to 0x001C and re-arms the object with state 0x19D1EE. No branches. */
#include "fn_auto.h"

int Fn_19D1DA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x44u, 0x0000u); /* 0x19D1DA */
    wr16(m, obj + 0x46u, 0x001Cu); /* 0x19D1E0 */
    wr32(m, obj + 0x00u, 0x0019D1EEu); /* 0x19D1E6 -> 0x19D1EE rts */
    return 0;
}
