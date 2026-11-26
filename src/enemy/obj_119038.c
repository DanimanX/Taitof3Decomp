#include "obj_119038.h"
#include "obj_E8B72.h"

/* ROM 0x119038: play sound 0x36, arm a 0x50-frame timer and hand over to
 * 0x119062. The +0x22 repeat count (6) is consumed by the 0x119068 loop
 * the later ticks re-enter; the `beq` at 0x119058 follows a move.w of a
 * non-zero constant, so it is never taken from here. */
int Obj_119038_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x22u, 0x0006u);
    Sound_PlayHooked(m, 0x0036u);                   /* 0x13F238 + 0x0039D8 */
    wr16(m, obj + 0x06u, 0x0050u);
    wr32(m, obj + 0x00u, 0x00119062u);              /* 0x11905A */
    return 0;
}
