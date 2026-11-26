/* ROM 0x137558: camera X countdown tick (A0 = object).
 * Decrements A5+0x30D2; while (int16_t) > 0x352 re-arms 0x137558. Otherwise
 * clears A5+0x30F3, calls Sound_PlayHooked(0x13), clears +0x56, sets
 * +0x20 = 0x8000 and +0x54 = 4, and transitions to 0x137594. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_137558(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t x = (uint16_t)(rd16(m, F3_A5 + 0x30D2u) - 1u); /* 0x137558 */
    wr16(m, F3_A5 + 0x30D2u, x);

    if ((int16_t)x > 0x0352) {                              /* 0x13755C..0x137562 */
        wr32(m, obj, 0x00137558u);                          /* 0x137550 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F3u, 0u);                            /* 0x137564 */
    Sound_PlayHooked(m, 0x0013u);                           /* 0x137568..0x137578 */
    wr16(m, obj + 0x56u, 0u);                               /* 0x13757C */
    wr16(m, obj + 0x20u, 0x8000u);                          /* 0x137580 */
    wr8(m, obj + 0x54u, 4u);                                /* 0x137586 */
    wr32(m, obj, 0x00137594u);                              /* 0x13758C */
    return 0;
}
