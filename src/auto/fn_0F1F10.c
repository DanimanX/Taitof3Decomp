/* ROM 0x0F1F10: turret/gun enemy initializer (A0 = object).
 * Seeds from 0x0F21A0 (0x0CEC4C), sets class colour (0x0CEE2C), installs
 * script 0x0F2210 (0x0CF1D8) and behaviour 0x0F226A (0x0D0678), clears bit 1
 * and sets bit 2 of +0x34, and arms 0x0F1F48. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

int Fn_0F1F10(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_SeedFromParams(m, obj, 0x000F21A0u);            /* 0x0F1F10..0x0F1F14 */
    Obj_ClassColour(m, obj);                            /* 0x0F1F1A */
    Obj_StartScript(m, obj, 0x000F2210u);               /* 0x0F1F20..0x0F1F24 */

    uint32_t a1 = 0x000F226Au;                          /* 0x0F1F2A */
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0F1F2E */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, a1 + 2u);
    r->a[1] = a1 + 8u;

    uint8_t f34 = rd8(m, obj + 0x34u);                  /* 0x0F1F34..0x0F1F3A */
    wr8(m, obj + 0x34u, (uint8_t)((f34 & ~0x02u) | 0x04u));
    wr32(m, obj, 0x000F1F48u);                          /* 0x0F1F40 */
    return 0;
}
