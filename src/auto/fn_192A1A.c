/* ROM 0x192A1A: scrolling boss-part initializer (A0 = object).
 * Clears common fields via 0x17A7FA, sets sprite pointer +0x10 = 0x0602EC,
 * copies initial position (+0x0C.w, +0x0E.w) into (+0x50.w, +0x54.w), and
 * arms 0x192A3C. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_192A1A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_17A7FA(m, obj);                                  /* 0x192A1A */
    wr32(m, obj + 0x10u, 0x000602ECu);                   /* 0x192A20 */
    wr16(m, obj + 0x50u, rd16(m, obj + 0x0Cu));          /* 0x192A28 */
    wr16(m, obj + 0x54u, rd16(m, obj + 0x0Eu));          /* 0x192A2E */
    wr32(m, obj + 0x00u, 0x00192A3Cu);                   /* 0x192A34 */
    return 0;                                            /* 0x192A3A */
}
