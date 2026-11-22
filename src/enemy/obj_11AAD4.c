#include "obj_11AAD4.h"
#include "../objects/obj_pool.h"

/* ROM 0x11AACC: timer still positive: stay on 0x11AAD4 */
static void sub_11AACC(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0011AAD4u);
}

/* ROM 0x11AAD4: after the +0x6 timer run the panel restore loop and chain
 * to 0x11AB3E */
int Obj_11AAD4_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        sub_11AACC(m, obj);                                          /* 0x11AAD8 */
        return 0;
    }
    /* 0x11AADA: bset.b #3,(0x1d36,A5) */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x08u));
    wr16(m, obj + 0x06u, 0x0010u);
    /* 0x11AAE6 beq.b tests Z from move.w, which only zeroes Z when the
     * loaded value is 0; 0x10 != 0 so 0x11AAF0 is unreachable here:
     *   move.l #0x11AAF0,(A0); rts */
    wr32(m, obj + 0x00u, 0x0011AAF0u);
    return 0;
}
