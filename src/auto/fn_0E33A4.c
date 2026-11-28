/* ROM 0x0E33A4: trail/chain segment initializer (A0 = object).
 * Seeds HP/hitboxes from the parameter record 0x0DC84E (0x0CEC4C), installs
 * the behaviour record 0x0E3400 (0x0D0678, which leaves D0 = its frame word
 * 0x0000 and A1 = record + 8), and arms tick 0x0E33C0. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E33A4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_SeedFromParams(m, obj, 0x000DC84Eu);            /* 0x0E33A4..0x0E33A8 */

    uint32_t a1 = 0x000E3400u;                          /* 0x0E33AE lea (0x50,PC) */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0E33B2 -> 0x0D0678 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a1 + 2u);
    r->a[1] = a1 + 8u;

    wr32(m, obj, 0x000E33C0u);                          /* 0x0E33B8 */
    return 0;                                           /* 0x0E33BE rts */
}
