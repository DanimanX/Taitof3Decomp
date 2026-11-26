/* ROM 0x19ADE4: boss articulated sub-part delay / anim init tick (A0 = object).
 * Decrements +0x06 and re-arms 0x19ADE4 while > 0. On <= 0, reads anim
 * index (0) and body ptr (0x19C17E) from 0x19C17C into +0x40/+0x3A, runs
 * 0x179A5C[idx].init, and transitions to 0x19AE0E. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"

int Fn_19ADE4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x19ADE4 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                  /* 0x19ADE8 */
        wr32(m, obj, 0x0019ADE4u);                           /* 0x19ADDC */
        return 0;
    }

    uint32_t a1 = 0x0019C17Cu;                               /* 0x19ADEA */
    uint16_t idx = rd16(m, a1);                              /* 0x19ADF0 */
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);                               /* 0x19ADF2 */
    wr32(m, obj + 0x3Au, a1);                                /* 0x19ADF6 */
    if (Obj_AnimInit(m, obj, idx) < 0)                       /* 0x19ADFA..0x19AE04 */
        return -1;
    r->d[0] = (r->d[0] & 0xFFFF0000u) | idx;
    r->a[1] = a1 + 6u;                                       /* 0x179550 advances A1 by 6 */
    wr32(m, obj, 0x0019AE0Eu);                               /* 0x19AE06 */
    return 0;
}
