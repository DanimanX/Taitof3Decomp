/* ROM 0x010638: register-level wrapper of Obj_ResetPools (src/objects/obj_pool.h) for its capture.
 * No arguments: groups 0..10 of the free mask (0x410000) and of the per-frame
 * snapshot (0x410030) are filled with 0xFFFFFFFF (0x01063C..0x01066A). The body
 * is bracketed by `movem.l d0-d1/a0-a1,-(a7)` / `movem.l (a7)+` (0x010638 /
 * 0x01066C), so every register except SP is unchanged at rts. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_010638(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    Obj_ResetPools(m);
    return 0;
}
