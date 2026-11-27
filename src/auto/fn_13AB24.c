/* ROM 0x13AB24: clears +0x06.w, copies the stage id A5+0x32D6 to +0x22.w and
 * arms 0x13AB36. */
#include "fn_auto.h"

int Fn_13AB24(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t stage = rd16(m, F3_A5 + 0x32D6u);          /* 0x13AB28 */

    wr16(m, obj + 0x06u, 0u);                           /* 0x13AB24 */
    wr16(m, obj + 0x22u, stage);                        /* 0x13AB28 */
    wr32(m, obj, 0x0013AB36u);                          /* 0x13AB2E */
    return 0;
}
