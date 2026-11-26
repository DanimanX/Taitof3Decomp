/* ROM 0x1380B4: child enemy/part init (A0 = object).
 * Clears per-frame fields via 0x0CBD86, sets +0x4C = 0x1388, copies +0x20 from
 * parent (+0x08), links +0x44 = param (+0x0C) and param->+0x48 = self, sets
 * +0x22 = param->+0x22 + 1, HP +0x24 = 0x28, dmg +0x28 = 1, hitbox [-9..9],
 * +0x42 = 0x30, and arms 0x13811A. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_1380B4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);                            /* 0x1380B4: jsr 0x0CBD86 */
    wr8(m, obj + 0x4Fu, 0u);                            /* 0x1380BA */
    wr16(m, obj + 0x4Cu, 0x1388u);                      /* 0x1380BE */

    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x1380C4 */
    wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));          /* 0x1380C8 */

    a1 = rd32(m, obj + 0x0Cu);                          /* 0x1380CE */
    wr32(m, obj + 0x44u, a1);                           /* 0x1380D2 */
    wr32(m, a1 + 0x48u, obj);                           /* 0x1380D6 */
    wr32(m, obj + 0x48u, 0u);                           /* 0x1380DA */
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));          /* 0x1380DE */
    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u)); /* 0x1380E4 */
    wr16(m, obj + 0x24u, 0x0028u);                      /* 0x1380E8 */
    wr16(m, obj + 0x28u, 0x0001u);                      /* 0x1380EE */
    wr16(m, obj + 0x2Cu, 0xFFF7u);                      /* 0x1380F4 */
    wr16(m, obj + 0x30u, 0xFFF7u);                      /* 0x1380FA */
    wr16(m, obj + 0x2Eu, 0x0009u);                      /* 0x138100 */
    wr16(m, obj + 0x32u, 0x0009u);                      /* 0x138106 */
    wr16(m, obj + 0x42u, 0x0030u);                      /* 0x13810C */
    wr32(m, obj, 0x0013811Au);                          /* 0x138112 */

    r->a[1] = a1;
    return 0;
}
