/* ROM 0x183308: boss sub-part offset initializer (A0 = object).
 * Inherits parent (+0x08) offset pos via 0x17BD68, seeds stats from 0x183352
 * via 0x184888, installs anim list 0x183368 (index 0 via 0x179A5C), installs
 * script10 0x183388, sets +0x34 = 2, and transitions to 0x17BE1C. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_183308(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_InheritOffsetPos(m, obj);                          /* 0x183308 -> 0x17BD68 */
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint32_t d1 = ((uint32_t)(int32_t)(int8_t)rd8(m, obj + 0x0Du) & 0xFFFF0000u)
                | rd16(m, obj + 0x54u);

    Obj_184888(m, obj, 0x00183352u);                       /* 0x18330C..0x183312 */
    uint32_t d0 = rd32(m, obj + 0x24u);

    uint32_t a1 = 0x00183368u;                             /* 0x183318 */
    uint16_t idx = rd16(m, a1);                            /* 0x18331E */
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);                             /* 0x183320 */
    wr32(m, obj + 0x3Au, a1);                              /* 0x183324 */
    if (Obj_AnimInit(m, obj, idx) < 0)                     /* 0x183328..0x183332 */
        return -1;

    a1 = 0x00183388u;                                      /* 0x183334 */
    wr32(m, obj + 0x44u, a1);                              /* 0x18333A */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));                /* 0x18333E */
    wr16(m, obj + 0x4Au, 0u);                              /* 0x183344 */
    wr8(m, obj + 0x34u, 2u);                               /* 0x183348 */
    wr32(m, obj, 0x0017BE1Cu);                             /* 0x18334E -> 0x17BE14 */

    r->d[0] = setw(d0, idx);
    r->d[1] = d1;
    r->a[1] = a1;
    r->a[4] = a4;
    return 0;
}
