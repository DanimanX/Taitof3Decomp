/* ROM 0x13DE08: queue palette fade-in ramp (0x00198E -> 0x0200C0, A30, 8 steps, 0x14 entries)
 * via 0x0018BA -> 0x001792, then play hooked sound 0x19 (if A5+0x32DE == 7) or 0x16. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"

int Fn_13DE08(const f3mem_t *m, f3regs_t *r)
{
    /* 0x13DE08..0x13DE26: jsr 0x0018BA(0x200C0, 0xA30, 8, 0x14) -> 0x001792 */
    uint32_t slot = Pal_QueueRamp(m, 0x0000198Eu, 0x000200C0u, 0x00000A30u, 8u, 0x14u);
    /* 0x0017C8 returns 0xFFFFFFFF (-1) when no ramp slot is available. */
    r->d[0] = (slot == 0u) ? 0xFFFFFFFFu : slot;

    if (rd16(m, F3_A5 + 0x32DEu) == 7u)                  /* 0x13DE2A..0x13DE30 */
        Sound_PlayHooked(m, 0x0019u);                    /* 0x13DE4C..0x13DE5C */
    else
        Sound_PlayHooked(m, 0x0016u);                    /* 0x13DE34..0x13DE44 */

    return 0;
}
