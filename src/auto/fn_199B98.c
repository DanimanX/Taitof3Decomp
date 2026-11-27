/* ROM 0x199B98: palette-animation object initialiser (A0 = object). Clears
 * +0x44/+0x4C, sets +0x48 = -1, queues two 4-word palette uploads through
 * 0x001660 (sources 0x199E20 -> 0x583C and 0x199E60 -> 0x584C) and re-arms the
 * object with the tick 0x199BE6. 0x001660 restores D0-D2/A0/A2, so the register
 * file is unchanged at rts. */
#include "fn_auto.h"
#include "../video/palette.h"

int Fn_199B98(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x44u, 0u);                        /* 0x199B98 clr.l (0x44,A0) */
    wr16(m, obj + 0x48u, 0xFFFFu);                   /* 0x199B9C move.w #-0x1,(0x48,A0) */
    wr32(m, obj + 0x4Cu, 0u);                        /* 0x199BA2 clr.l (0x4c,A0) */
    /* 0x199BA6 move.l #4,-(SP) ; 0x199BAC move.l #0x583c,-(SP) ;
     * 0x199BB2 move.l #0x199e20,-(SP) ; 0x199BB8 jsr 0x001660 */
    Pal_QueueUpload(m, 0x00199E20u, 0x583Cu, 4u);
    /* 0x199BC2..0x199BD4: the same with 0x199e60 / 0x584c */
    Pal_QueueUpload(m, 0x00199E60u, 0x584Cu, 4u);

    wr32(m, obj, 0x00199BE6u);                       /* 0x199BDE move.l #0x199be6,(A0) */
    return 0;                                        /* 0x199BE4 rts */
}
