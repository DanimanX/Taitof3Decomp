/* ROM 0x003998: register-level wrapper of Sound_WriteScratchBase
 * (src/audio/sound_cmd.c) for its capture.
 *   003998 lea -$13c2(a5),a2 ; 00399C moveq #$0,d0 ; 00399E move.b (a2)+,d0
 *   0039A0 move.b d0,(a1,d1.w) ; 0039A4 subq.w #$1,d0 ; ... 0039B6 rts
 * Destination base = the caller's A1 (the capture's injected call has it at
 * 0x41005C, so the ring bytes land in Work RAM). D0 ends at 0x0000FFFF (the
 * moveq cleared the whole long, the dbra expired to -1); D1 keeps its entry
 * upper half and gets the wrapped index; A2 ends at A5-0x13C2 + bytes read. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"

int Fn_003998(const f3mem_t *m, f3regs_t *r)
{
    uint16_t idx = (uint16_t)r->d[1];
    uint16_t used = Sound_WriteScratchBase(m, r->a[1], &idx);

    r->d[0] = 0x0000FFFFu;                          /* 0x0039B2 dbra d0 */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | idx;        /* addq.w/andi.w only */
    r->a[2] = (F3_A5 - 0x13C2u) + used;             /* 0x00399E/0x0039A8 (a2)+ */
    return 0;                                       /* 0x0039B6 rts */
}
