#include "obj_BDB1A.h"
#include "../objects/obj_common.h"
#include "../video/palette.h"

/* ROM 0x0BDB1A: one-shot palette init. Queues one 0x1866 fade-out and eight
 * 0x18BA ramps (D0 = 0x20/0x8, D1/A1 = table entry from 0x198E, D2 = 0x20),
 * then re-arms itself to 0x0BDC5C, which falls through to the plain rts. */

/* ROM 0x18BA: ramp slot A1 = (0x8,A6) with D0/D1/D2 = (0xC,A6)/(0x10,A6)/
 * (0x14,A6); the PC-relative table pointer is 0x18D4 + 0xBA = 0x198E. */
static uint32_t lab_0018BA(const f3mem_t *m, uint32_t a1, uint32_t d0,
                           uint32_t d1, uint32_t d2)
{
    return Pal_QueueRamp(m, 0x0000198Eu, a1, d0, d1, d2);   /* lea (0xBA,PC),A0 = 0x198E */
}

int Obj_BDB1A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BDB36: jsr 0x1866 with A1 = 0x26CC0 */
    /* cdecl-style: (8,A6)=A1, (0xC)=D0, (0x10)=D1, (0x14)=D2; pushed last-first */
    Pal_FadeOut001866(m, 0x00026CC0u, 0x1380u, 0x8u, 0x20u);

    /* 0x0BDB40..0x0BDC46: eight 0x18BA calls, (D1, arg3) pairs in ROM order */
    static const uint32_t ramp_args[8][2] = {
        { 0x00022240u, 0x1700u },
        { 0x00022040u, 0x1500u },
        { 0x00022740u, 0x1C00u },
        { 0x00022340u, 0x1800u },
        { 0x000226C0u, 0x1B80u },
        { 0x00022640u, 0x1B00u },
        { 0x000224C0u, 0x1980u },
        { 0x000220C0u, 0x1580u },
    };
    for (unsigned i = 0; i < 8u; i++) {
        (void)lab_0018BA(m, ramp_args[i][0], ramp_args[i][1], 0x8u, 0x20u);
    }

    /* 0x0BDC50: movem.l (SP)+,A0 ; move.l #0xBDC5C,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x000BDC5Cu);
    return 0;
}
