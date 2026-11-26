/* ROM 0x123550: link child segment at tail of parent (+0x08) +0x44/+0x48 chain (A0 = object).
 * Sets +0x20 = 0x8000, copies +0x22 from parent and increments parent+0x22,
 * walks (+0x08)->+0x48 to the tail node A1, links A0 (+0x44 = A1, A1+0x48 = A0,
 * +0x48 = 0), sets +0x34 = 0x1400 (bytes +0x34=0x14, +0x35=0), clears +0x1C/+0x1E
 * and byte +0x4F. */
#include "fn_auto.h"

int Fn_123550(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x20u, 0x8000u);                       /* 0x123550 */
    uint32_t a1 = rd32(m, obj + 0x08u);                  /* 0x123556 */
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));           /* 0x12355A */
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) + 1u)); /* 0x123560 */
    wr32(m, obj + 0x44u, 0u);                            /* 0x123566 */
    wr32(m, obj + 0x48u, 0u);                            /* 0x12356A */

    for (;;) {                                           /* 0x12356E */
        uint32_t next = rd32(m, a1 + 0x48u);
        if (next == 0u)
            break;
        a1 = next;
    }

    wr32(m, obj + 0x44u, a1);                            /* 0x12357A */
    wr32(m, a1 + 0x48u, obj);                            /* 0x12357E */
    wr16(m, obj + 0x34u, 0u);                            /* 0x123582 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x14u)); /* 0x123586, 0x12358C */
    wr16(m, obj + 0x1Eu, 0u);                            /* 0x123592 */
    wr16(m, obj + 0x1Cu, 0u);                            /* 0x123596 */
    wr8(m, obj + 0x4Fu, 0u);                             /* 0x12359A */

    r->d[0] = 0u;
    r->a[1] = a1;
    return 0;
}
