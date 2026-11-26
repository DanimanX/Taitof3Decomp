/* ROM 0x17FA66: delayed boss sub-part activation tick (A0 = object).
 * Counts down +0x06 and re-arms 0x17FA66 while > 0. On 0, seeds stats via
 * 0x184888(0x17FAF6), selects anim list 0x17FB0C (+0x20.b < 0) or 0x17FB5C,
 * runs 0x179A5C anim init, loads script +0x0C from table 0x17FACE[+0x0E.b],
 * sets +0x06 = 0x40, and transitions to 0x17C14C. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

int Fn_17FA66(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x17FA66 */
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer > 0) {                               /* 0x17FA6A */
        wr32(m, obj, 0x0017FA66u);                          /* 0x17FA5E */
        return 0;
    }

    Obj_184888(m, obj, 0x0017FAF6u);                        /* 0x17FA6C..0x17FA72 */
    uint32_t a1 = ((int8_t)rd8(m, obj + 0x20u) < 0) ? 0x0017FB0Cu : 0x0017FB5Cu; /* 0x17FA78..0x17FA8A */

    uint16_t idx = rd16(m, a1);                             /* 0x17FA8E */
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);                              /* 0x17FA90 */
    wr32(m, obj + 0x3Au, a1);                               /* 0x17FA94 */
    if (Obj_AnimInit(m, obj, idx) < 0)                      /* 0x17FA98..0x17FAA2 */
        return -1;

    uint32_t d0 = (uint32_t)rd8(m, obj + 0x0Eu);            /* 0x17FAA4: bfextu (0xc,A0){16:8},D0 */
    uint32_t a2 = 0x0017FACEu + (uint32_t)(int32_t)(int16_t)d0 * 8u; /* 0x17FAAA */
    a1 = rd32(m, a2); a2 += 4u;                             /* 0x17FAB0 */
    wr32(m, obj + 0x44u, a1);                               /* 0x17FAB2 */
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));                 /* 0x17FAB6 */
    wr16(m, obj + 0x4Au, 0u);                               /* 0x17FABC */
    wr32(m, obj + 0x0Cu, rd32(m, a2));                      /* 0x17FAC0 */
    wr16(m, obj + 0x06u, 0x0040u);                          /* 0x17FAC4 */
    wr32(m, obj, 0x0017C14Cu);                              /* 0x17FACA -> 0x17C144 */

    r->d[0] = d0;
    r->a[1] = a1;
    r->a[2] = a2 + 4u;
    return 0;
}
