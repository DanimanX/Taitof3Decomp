/* ROM 0x12D542: object countdown step. Decrements the word at +0x06; while it
 * stays positive the object re-arms itself with 0x12D542, otherwise it resolves
 * the pointer (A5+0x32AA)+0x48 and seeds its motion fields (+0x56 = -0x100000,
 * +0x5A = 0x500000, +0x5E = 0) before arming 0x12D570. A1 keeps the resolved
 * pointer. */
#include "fn_auto.h"

int Fn_12D542(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x12D542 */
    wr16(m, obj + 0x06u, (uint16_t)n);

    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0012D542u);  /* 0x12D546 bgt -> 0x12D53A */
        return 0;
    }

    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu); /* 0x12D548 movea.l (0x32AA,A5),A1 */
    a1 = rd32(m, a1 + 0x48u);               /* 0x12D54C movea.l (0x48,A1),A1 */
    wr32(m, a1 + 0x56u, 0xFFF00000u);       /* 0x12D550 #-0x100000 */
    wr32(m, a1 + 0x5Au, 0x00500000u);       /* 0x12D558 */
    wr32(m, a1 + 0x5Eu, 0x00000000u);       /* 0x12D560 */
    wr32(m, obj + 0x00u, 0x0012D570u);      /* 0x12D568 */
    r->a[1] = a1;
    return 0;
}
