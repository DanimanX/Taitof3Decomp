/* ROM 0x1131EA: orbiter child bring-up (A0 = object, A0+0x08 = parent).
 * Plays sound 0x2E, clears the per-frame fields, sets speed +0x42 = 0x20 and
 * falls through into the shared body at 0x11320A (whose C translation is
 * Fn_11320A, verified against its own capture with the same entry state). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"

int Fn_1131EA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Sound_PlayHooked(m, 0x002Eu);       /* ROM 0x1131EA: move.w #0x2e,-(SP); 0x13F238 + 0x0039D8 */
    Obj_ResetFields(m, obj);            /* ROM 0x1131FE: jsr 0x000CBD86 */
    wr16(m, obj + 0x42u, 0x0020u);      /* ROM 0x113204: move.w #0x20,(0x42,A0) */

    return Fn_11320A(m, r);             /* ROM 0x11320A: falls through */
}
