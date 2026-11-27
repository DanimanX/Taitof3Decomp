#include "fn_auto.h"
/* ROM 0x112F42: UNVERIFIED (no capture): Wait for a linked object's flag or clear its field on timeout. */

int Fn_112F42(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[1] = rd32(m, obj + 8u); /* 0x112F42 */
    if ((rd8(m, r->a[1] + 0x50u) & 0x10u) == 0u) {
        uint16_t count = rd16(m, obj + 6u);
        wr16(m, obj + 6u, (uint16_t)(count - 1u)); /* 0x112F50 */
        /* BGT uses N xor V from SUBQ, including when count is 0x8000. */
        if ((int16_t)count > 1) {
            wr32(m, obj, 0x112F42u); /* 0x112F3A */
            return 0;
        }

        r->a[1] = rd32(m, obj + 8u); /* 0x1131AE */
        wr32(m, r->a[1] + 0x24u, 0u);
    }

    wr32(m, obj, 0x1131A0u); /* 0x113198 */
    return 0;
}
