/* ROM 0x0F1B04: turret/gun enemy initializer (A0 = object).
 * Seeds from 0x0F16AC (0x0CEC4C), installs script 0x0F1CEE (0x0CF1D8) and
 * behaviour 0x0F1762 (0x0D0678), sets bit 2 of +0x34, and arms 0x0F1B30. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

int Fn_0F1B04(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_SeedFromParams(m, obj, 0x000F16ACu);            /* 0x0F1B04..0x0F1B08 */
    Obj_StartScript(m, obj, 0x000F1CEEu);               /* 0x0F1B0E..0x0F1B12 */

    uint32_t a1 = 0x000F1762u;                          /* 0x0F1B18 */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F1B1C */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a1 + 2u);
    r->a[1] = a1 + 8u;

    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u)); /* 0x0F1B22 */
    wr32(m, obj, 0x000F1B30u);                          /* 0x0F1B28 */
    return 0;
}
