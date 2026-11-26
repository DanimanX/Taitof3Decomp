/* ROM 0x0CCE0E: sprite/anim initializer thunk (A0 = object).
 * Loads A1 = 0x0CCE16 and jumps to 0x0CCC5A: stores A1 into +0x3A, sets
 * +0x1E = 0xC0C0, +0x42 = 0x1010, +0x06 = 0x000C, +0x10 = 0x00062D98,
 * clears +0x1C, and arms 0x0CCCA2. */
#include "fn_auto.h"

int Fn_0CCE0E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x000CCE16u;                          /* 0x0CCE0E */

    wr32(m, obj + 0x3Au, a1);                           /* 0x0CCC5A */
    wr16(m, obj + 0x1Eu, 0xC0C0u);                      /* 0x0CCC5E */
    wr16(m, obj + 0x42u, 0x1010u);                      /* 0x0CCC64 */
    wr16(m, obj + 0x06u, 0x000Cu);                      /* 0x0CCC6A */
    wr32(m, obj + 0x10u, 0x00062D98u);                  /* 0x0CCC8E */
    wr16(m, obj + 0x1Cu, 0u);                           /* 0x0CCC96 */
    wr32(m, obj, 0x000CCCA2u);                          /* 0x0CCC9A */

    r->a[1] = a1;
    return 0;
}
