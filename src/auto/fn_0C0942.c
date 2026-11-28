/* ROM 0x0C0942: falling debris/particle tick (A0 = object).
 * Steps the {dur.w, sprdef.l} animation table 0x0C095E (0x0CBD52), draws the
 * sprite (0x001282), shifts the long Y (+0x18) up by 0x8000 (screen-space
 * drift) and re-arms 0x0C0942 through the 0x0C093A thunk. */
#include "fn_auto.h"
#include "batch_obj_regs.h"
#include "../objects/obj_common.h"

int Fn_0C0942(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    r->a[1] = 0x000C095Eu;                              /* 0x0C0942 lea (0x1A,PC) */
    Obj_Animate(m, obj, r->a[1]);                       /* 0x0C0948 -> 0x0CBD52 */
    batch_draw_sprite(m, r, 0x000C0954u);               /* 0x0C094E -> 0x001282 */

    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x00008000u); /* 0x0C0954 subi.l */
    wr32(m, obj, 0x000C0942u);                          /* 0x0C095C -> 0x0C093A */
    return 0;                                           /* 0x0C0940 rts */
}
