/* ROM 0x00154DD0: register-level wrapper of Pal_UploadFade154DD0
 * (src/video/palette.c) for its capture.
 * The body is 16x { movem.l d0-d3/a0-a2,-(sp) ; blend/queue ; movem.l (sp)+ }
 * pairs ending in rts, so D0-D3 and A0-A2 are restored by the ROM itself.
 * Pal_Blend64 (0x16A258) is not inside that save set: its move.w #0x3f,d7 /
 * dbra loop leaves D7 = (entry D7 & 0xFFFF0000) | 0xFFFF after every pair,
 * which is the only register the ROM returns changed. */
#include "fn_auto.h"
#include "../video/palette.h"

int Fn_154DD0(const f3mem_t *m, f3regs_t *r)
{
    Pal_UploadFade154DD0(m);
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0xFFFFu;
    return 0;
}
