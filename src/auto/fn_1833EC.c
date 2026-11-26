/* ROM 0x1833EC: boss sub-part delayed activation tick (A0 = object).
 * Counts down +0x06; while > 0 re-arms 0x1833EC. When <= 0, seeds stats from
 * 0x183438 via 0x184888, installs anim list 0x18390C (index 2 via 0x179A5C),
 * installs script10 0x18345C, sets +0x34 = 2, and transitions to 0x17BE1C. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1833EC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x1833EC */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                /* 0x1833F0 */
        wr32(m, obj, 0x001833ECu);                         /* 0x1833E4 */
        return 0;
    }

    Obj_184888(m, obj, 0x00183438u);                       /* 0x1833F2..0x1833F8 */

    uint32_t a1 = 0x0018390Cu;                             /* 0x1833FE */
    uint16_t idx = rd16(m, a1);                            /* 0x183404 */
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);                             /* 0x183406 */
    wr32(m, obj + 0x3Au, a1);                              /* 0x18340A */
    if (Obj_AnimInit(m, obj, idx) < 0)                     /* 0x18340E..0x183418 */
        return -1;

    uint32_t d0 = (uint32_t)(uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3; /* 0x1798BA..0x1798C4 */

    a1 = 0x0018345Cu;                                      /* 0x18341A */
    wr32(m, obj + 0x44u, a1);                              /* 0x183420 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));                /* 0x183424 */
    wr16(m, obj + 0x4Au, 0u);                              /* 0x18342A */
    wr8(m, obj + 0x34u, 2u);                               /* 0x18342E */
    wr32(m, obj, 0x0017BE1Cu);                             /* 0x183434 -> 0x17BE14 */

    r->d[0] = setw(0u, (uint16_t)d0);
    r->a[1] = a1;
    return 0;
}
