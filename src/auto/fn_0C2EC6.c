#include "fn_auto.h"
/* ROM 0x0C2EC6: countdown gate for the stage-2 line-RAM band (A0 = object).
 * +0x06 counts down one per call; while it stays > 0 the tick re-arms itself
 * (thunk 0x0C2EBE) and nothing else happens. On expiry it starts a palette
 * fade-in of 0x20 entries from ROM 0x201C0 to palette index 0x700 over 0x3C
 * frames (0x001890 with A1 = 0x190E as the "from" frame), claiming a ramp slot
 * through 0x001792; D0 leaves with that slot address (or 0 when all 16 slots
 * are busy, which is what 0x001792 returns), then +0x06 is reloaded with 0x3C
 * and the object is re-armed at 0x0C2F06 (the same countdown, but its expiry
 * path frees the object). */
#include "../objects/obj_common.h"   /* Pal_QueueRamp (0x001792) */

int Fn_0C2EC6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t old = rd16(m, obj + 0x06u);
    uint16_t cnt = (uint16_t)(old - 1u);                   /* 0x0C2EC6 subq.w #1,(6,A0) */
    wr16(m, obj + 0x06u, cnt);
    /* 0x0C2ECA bgt 0x0C2EBE: signed > 0, except when the subtraction
     * overflowed (old = 0x8000), where N != V and bgt is false. */
    if ((int16_t)cnt > 0 && old != 0x8000u) {
        wr32(m, obj, 0x000C2EC6u);                         /* 0x0C2EBE */
        return 0;                                          /* 0x0C2EC4 rts */
    }

    /* 0x0C2ECC..0x0C2EF0: movem {A0} + four longs of arguments to 0x001890,
     * which queues the ramp from 0x190E to 0x201C0, index 0x700, 0x3C frames,
     * 0x20 entries, and returns its slot (0x0018B0 move.l A0,D0). */
    r->d[0] = Pal_QueueRamp(m, 0x000201C0u, 0x0000190Eu, 0x0700u, 0x003Cu, 0x0020u);

    wr16(m, obj + 0x06u, 0x003Cu);                         /* 0x0C2EF6 (Z = 0 from here on) */
    /* 0x0C2EFC beq.b 0x0C2F0C is never taken: move.w #0x3C clears Z. The
     * target 0x0C2F0C is the free path of the 0x0C2F06 entry (src/auto/
     * fn_0C2F06.c), not of this one. */
    wr32(m, obj, 0x000C2F06u);                             /* 0x0C2EFE */
    return 0;                                              /* 0x0C2F04 rts */
}
