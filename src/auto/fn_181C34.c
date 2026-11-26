/* ROM 0x181C34: delayed boss/enemy phase activation tick (A0 = object).
 * Counts down +0x06 and re-arms 0x181C34 while > 0. On 0, plays hooked sound
 * 0x2A, seeds stats via 0x184888(0x181CDE), starts anim 0x181CF4 via
 * 0x179A5C, sets script 0x181D20 (+0x44/+0x48/+0x4A), stores 0x181C96 to +0x0C,
 * and transitions to 0x17C14C. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_181C34(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x181C34 */
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer > 0) {                               /* 0x181C38 */
        wr32(m, obj, 0x00181C34u);                          /* 0x181C2C */
        return 0;
    }

    Sound_PlayHooked(m, 0x002Au);                           /* 0x181C3A..0x181C4A */
    Obj_184888(m, obj, 0x00181CDEu);                        /* 0x181C4E..0x181C54 */
    uint32_t d0 = rd32(m, obj + 0x24u);

    uint32_t a1 = 0x00181CF4u;                              /* 0x181C5A */
    uint16_t idx = rd16(m, a1);                             /* 0x181C60 */
    a1 += 2u;
    d0 = setw(d0, idx);
    wr16(m, obj + 0x40u, idx);                              /* 0x181C62 */
    wr32(m, obj + 0x3Au, a1);                               /* 0x181C66 */
    if (Obj_AnimInit(m, obj, idx) < 0)                      /* 0x181C6A..0x181C74 */
        return -1;

    a1 = 0x00181D20u;                                       /* 0x181C76 */
    wr32(m, obj + 0x44u, a1);                               /* 0x181C7C */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));                 /* 0x181C80 */
    wr16(m, obj + 0x4Au, 0u);                               /* 0x181C86 */
    wr32(m, obj + 0x0Cu, 0x00181C96u);                      /* 0x181C8A */
    wr32(m, obj, 0x0017C14Cu);                              /* 0x181C92 -> 0x17C144 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
