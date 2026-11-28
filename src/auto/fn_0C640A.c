/* ROM 0x0C640A: register-level wrapper of Coll_ResetLists (src/collision/collision.h) for its capture.
 * No arguments; the six cursors around 0x4088FE..0x408912 and the two area-damage
 * words at 0x41A980 are (re)installed by the move.l block at 0x0C640A..0x0C642C.
 * The tail 0x0C6434..0x0C6440 also leaves two registers behind:
 *   A0 = 0x41A980 + 2*4 = 0x41A988  (0x0C6434 lea, then two post-increments)
 *   D0 = 0xFFFF in the low word only  (0x0C643A `move.w #$1,d0` + two `dbra`
 *        decrements; the upper half keeps the entry's bits, e.g. 0x0041ACE0 ->
 *        0x0041FFFF in pair 0)
 * D1 and every other register are untouched. */
#include "fn_auto.h"
#include "../collision/collision.h"

int Fn_0C640A(const f3mem_t *m, f3regs_t *r)
{
    Coll_ResetLists(m);
    r->a[0] = 0x0041A988u;                                  /* 0x0C6434..0x0C6440 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x0000FFFFu;
    return 0;
}
