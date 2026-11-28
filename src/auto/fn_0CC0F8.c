/* ROM 0x0CC0F8: register-level wrapper of Obj_CC0F4_Tick (src/enemy/obj_CC0F4.c, its 0x0CC0F4 entry branches here) for its capture.
 * The routine takes its object in A0 and is `movea.l $8(a0),a1` /
 * `subq.w #$1,$54(a1)` / `jsr $105ec` (Obj_Free) / `rts`: no stack arguments and
 * no further registers touched. A1 is left pointing at the object's parent
 * (A0+8) — it is the only register that changes, and the capture shows it varying
 * per pair, so it is read from memory rather than assumed constant. Obj_Free
 * only clears a free-mask bit, so the parent pointer is the same before and after
 * the call; it is read first, exactly as 0x0CC0F8 does. */
#include "fn_auto.h"
#include "../enemy/obj_CC0F4.h"

int Fn_0CC0F8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t parent = rd32(m, r->a[0] + 8u);                /* 0x0CC0F8 movea.l $8(a0),a1 */
    Obj_CC0F4_Tick(m, r->a[0]);
    r->a[1] = parent;
    return 0;
}
