#include "fn_auto.h"
/* ROM 0x12C31E: Count down before returning the object to its sound-trigger wait. */

int Fn_12C31E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t delay = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(delay - 1u)); /* 0x12C31E */
    /* 0x12C322 returns through 0x12C316, or branches via 0x12C324 to 0x12C2E2. */
    wr32(m, obj, (int16_t)delay > 1 ? 0x12C31Eu : 0x12C2EAu);
    r->a[7] += 4u;
    return 0;
}
