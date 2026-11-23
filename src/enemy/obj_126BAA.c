#include "obj_126BAA.h"
#include "obj_11B262.h"

/* ROM 0x126BAA: steering tick. When the countdown at +0x06 runs out both
 * flags A5+0x30EE{0x13:1} and {0x10:1} are cleared and, continuing into
 * 0x1269A2, the turn is retargeted to +0x54 = 8 at (+0x50,+0x52) = (0xC0,0x30)
 * with a 0x78-frame timer, handing over to 0x1269C4. */
int Obj_126BAA_Tick(const f3mem_t *m, uint32_t obj)
{
    ObjHelper_11B288(m, obj);                      /* 0x126BAA jsr 0x11B288 */
    ObjHelper_11B2CA(m, obj);                      /* 0x126BB0 jsr 0x11B2CA */
    ObjHelper_11B262(m, obj);                      /* 0x126BB6 jsr 0x11B262 */

    /* 0x126BBC: move.w (0x6,A0),D0; andi.w #0xFF,D0; bne 0x126BC8 -- the
     * branch lands on the next instruction, so both paths continue at 0x126BC8
     * and neither leaves an observable side effect. */
    (void)((uint16_t)rd16(m, obj + 0x06u) & 0x00FFu);

    /* 0x126BC8: subq.w #1 -> bgt 0x126BA2 tests the NEW value */
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00126BAAu);         /* 0x126BA2 */
        return 0;
    }
    /* 0x126BCE: bfclr (0x30EE,A5){0x13:1}, then {0x10:1} */
    {
        uint32_t a13 = F3_A5 + 0x30EEu + 0x13u / 8u;
        wr8(m, a13, (uint8_t)(rd8(m, a13) & (uint8_t)~(0x80u >> (0x13u % 8u))));
        uint32_t a10 = F3_A5 + 0x30EEu + 0x10u / 8u;
        wr8(m, a10, (uint8_t)(rd8(m, a10) & (uint8_t)~(0x80u >> (0x10u % 8u))));
    }

    /* 0x126BDA: bra 0x1269A2. 0x1269A2 is not a routine of its own (the ROM
     * falls into it from 0x126BCE), so the reachable tail is translated here
     * and no callee is duplicated. */
    wr8(m, obj + 0x54u, 0x08u);                    /* 0x1269A2 */
    wr16(m, obj + 0x50u, 0x00C0u);                 /* 0x1269A8 */
    wr16(m, obj + 0x52u, 0x0030u);                 /* 0x1269AE */
    wr16(m, obj + 0x06u, 0x0078u);                 /* 0x1269B4 */
    /* 0x1269BA: beq 0x1269DC tests the Z set by move.w of 0x78: not taken */
    wr32(m, obj + 0x00u, 0x001269C4u);             /* 0x1269BC */
    return 0;
}
