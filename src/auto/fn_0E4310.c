/* ROM 0x0E4310: second entry of the trail/chain segment initializer
 * (A0 = object). Identical to 0x0E33A4 -- seed from 0x0DC84E (0x0CEC4C),
 * behaviour record 0x0E3400 (0x0D0678: D0 = 0x0000, A1 = 0x0E3408) -- but it
 * arms tick 0x0E432C instead of 0x0E33C0. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E4310(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_SeedFromParams(m, obj, 0x000DC84Eu);            /* 0x0E4310..0x0E4314 */

    uint32_t a1 = 0x000E3400u;                          /* 0x0E431A lea (-0xF1C,PC) */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0E431E -> 0x0D0678 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a1 + 2u);
    r->a[1] = a1 + 8u;

    wr32(m, obj, 0x000E432Cu);                          /* 0x0E4324 */
    return 0;                                           /* 0x0E432A rts */
}
