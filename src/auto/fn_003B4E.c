/* ROM 0x003B4E: register-level wrapper of Sound_CopyToRing
 * (src/audio/sound_cmd.c) for its capture.
 *   003B4E moveq #$0,d0 ; 003B50 move.b (a0)+,d0 ; 003B54 move.b (a0)+,(a1,d1.w)
 *   003B62 rts
 * The only register the body changes is A0 (one (a0)+ per byte, count byte
 * included); D0 keeps its entry upper half with 0xFFFF/0 in the low word and
 * D1 its entry upper half with the wrapped index. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003B4E(const f3mem_t *m, f3regs_t *r)
{
    uint16_t idx = (uint16_t)r->d[1];
    uint16_t used = Sound_CopyToRing(m, r->a[0], r->a[1], &idx);

    r->a[0] += used;                                 /* 0x003B50/0x003B54 (a0)+ */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0xFFFFu;     /* 0x003B5E dbra d0 */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | idx;
    return 0;                                        /* 0x003B62 rts */
}
