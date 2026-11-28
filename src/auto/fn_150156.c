/* ROM 0x00150156: register-level wrapper of Pal_UploadFade150156
 * (src/video/palette.c) for its capture.
 * The body is 16x { movem.l d0-d3/a0-a2,-(sp) ; blend/queue ; movem.l (sp)+ }
 * pairs ending in rts, so D0-D3 and A0-A2 survive. D7 is not in that movem:
 * Pal_Blend64 (0x16A258) sets D7.w = 0x3F and its dbra leaves 0xFFFF, so the
 * word half of D7 changes and the upper half survives. */
#include "fn_auto.h"
#include "../video/palette.h"

int Fn_150156(const f3mem_t *m, f3regs_t *r)
{
    Pal_UploadFade150156(m);
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0xFFFFu;   /* 0x16A25C dbra d7 */
    return 0;
}
