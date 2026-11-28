/* ROM 0x0B73E2: register-level wrapper of sub_0B73E2 (src/auto/fn_0B760C.c)
 * for its capture.
 * Stack (entry SP = S, return address at S): 0x0B73E2 first pushes six longs
 * (movem.l d0-d3/a0-a1), so a7 = S-0x18 and the arguments the body reads are
 * $1c(a7) = the word at S+4 (w), $1e(a7) = the word at S+6 (h) and $20(a7) =
 * the long at S+8 (rect, top-left {x.w, y.w} of the text layer). The movem is
 * unwound before the rts, so the caller's D0-D3/A0/A1 come back unchanged. */
#include "fn_auto.h"
#include "fn_helpers.h"

int Fn_0B73E2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint16_t w = rd16(m, sp + 4u);                 /* 0x0B73E6 move.w $1c(a7),d2 */
    uint16_t h = rd16(m, sp + 6u);                 /* 0x0B73EA move.w $1e(a7),d3 */
    uint32_t rect = rd32(m, sp + 8u);              /* 0x0B73EE movea.l $20(a7),a0 */

    sub_0B73E2(m, w, h, rect);
    return 0;                                      /* 0x0B741E rts */
}
