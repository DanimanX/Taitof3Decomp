/* ROM 0x17FE9A: delayed enemy/boss activation countdown tick (A0 = object).
 * Decrements +0x06 and re-arms 0x17FE9A while > 0. Once <= 0, plays sound
 * 0x11 via Sound_PlayHooked, seeds stats from 0x17FF3C (0x184888), initializes
 * anim list at 0x17FF52 (0x179A5C[0]), sets script 0x17FF72 into +0x44/+0x48,
 * scales HP by difficulty (0x18495E), sets +0x0C = 0x17FF02, arms 0x17C14C. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_17FE9A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x17FE9A */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                             /* 0x17FE9E */
        wr32(m, obj, 0x0017FE9Au);                      /* 0x17FE92 */
        return 0;
    }

    Sound_PlayHooked(m, 0x11u);                         /* 0x17FEA0..0x17FEB0 */
    Obj_184888(m, obj, 0x0017FF3Cu);                    /* 0x17FEB4 */
    uint32_t d0 = rd32(m, obj + 0x24u);                 /* D0 after 0x184888 divul.l */

    uint32_t a1 = 0x0017FF52u;                          /* 0x17FEC0 */
    uint16_t anim_idx = rd16(m, a1);                    /* 0x17FEC6 */
    a1 += 2u;
    d0 = (d0 & 0xFFFF0000u) | anim_idx;
    wr16(m, obj + 0x40u, anim_idx);                     /* 0x17FEC8 */
    wr32(m, obj + 0x3Au, a1);                           /* 0x17FECC */
    if (Obj_AnimInit(m, obj, anim_idx) < 0)             /* 0x17FED0..0x17FEDA */
        return -1;

    a1 = 0x0017FF72u;                                   /* 0x17FEDC */
    wr32(m, obj + 0x44u, a1);                           /* 0x17FEE2 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));          /* 0x17FEE6 */
    wr16(m, obj + 0x4Au, 0u);                           /* 0x17FEEC */
    Obj_18495E(m, obj);                                 /* 0x17FEF0: preserves D0/D1 */
    wr32(m, obj + 0x0Cu, 0x0017FF02u);                  /* 0x17FEF6 */
    wr32(m, obj, 0x0017C14Cu);                          /* 0x17FEFE -> 0x17C144 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
