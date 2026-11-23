#include "obj_11AAB8.h"

/* ROM 0x11AAB0: still counting down: stay on 0x11AAB8 */
static void sub_11AAB0(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0011AAB8u);
}

/* ROM 0x11AAB8: two-step hold (0x20 then 0x10 frames) before announcing the
 * global flags 2/3 and restoring the row; chains to 0x11AB3E */
int Obj_11AAB8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* subq.w #1,(0x6,A0) only clears Z on a 16-bit zero result, so bgt.b
     * tests the signed word */
    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        sub_11AAB0(m, obj);                              /* 0x11AABC */
        return 0;
    }

    /* 0x11AABE: bset.b #2,(0x1d36,A5) */
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x04u));
    wr16(m, obj + 0x06u, 0x0020u);
    /* 0x11AACA beq.b tests Z from move.w #0x20 (Z clear), so the
     * fall-through at 0x11AACC (chain to 0x11AAD4) is taken and the +0x6
     * reload is consumed by that entry. */
    wr32(m, obj + 0x00u, 0x0011AAD4u);
    return 0;
}
