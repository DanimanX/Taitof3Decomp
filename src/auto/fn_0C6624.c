/* ROM 0x0C6624: register-level wrapper of Terrain_SyncScroll (src/collision/collision.h) for its capture.
 * No arguments: two word copies from A5-relative globals (0x0C6624/0x0C662A copy
 * -0x5DA8(A5) = 0x402258 and -0x5DA4(A5) = 0x40225C to 0x918/0x91A(A5)). Both
 * instructions are `move.w` into memory only, so the register file is unchanged
 * at rts. */
#include "fn_auto.h"
#include "../collision/collision.h"

int Fn_0C6624(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    Terrain_SyncScroll(m);
    return 0;
}
