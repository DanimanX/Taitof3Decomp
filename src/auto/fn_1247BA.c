/* ROM 0x1247BA: boss/event countdown tick (A0 = object).
 * Decrements +0x06.w; while > 0 re-arms 0x1247BA. When <= 0, clears
 * A5+0x30F3, plays hooked sound 0x13 (0x13F238 + 0x0039D8), sets bit 0 of
 * A5+0x30F2, initializes +0x22/+0x20/+0x42/+0x3E/+0x3A, and transitions to
 * 0x124800. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_1247BA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);          /* 0x1247BA */
    wr16(m, obj + 0x06u, cnt);

    if ((int16_t)cnt > 0) {                                        /* 0x1247BE */
        wr32(m, obj, 0x001247BAu);                                 /* 0x1247B2 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F3u, 0u);                                   /* 0x1247C0 */
    Sound_PlayHooked(m, 0x0013u);                                  /* 0x1247C4..0x1247D4 (0x13F238 + 0x0039D8) */
    wr8(m, F3_A5 + 0x30F2u, (uint8_t)(rd8(m, F3_A5 + 0x30F2u) | 0x01u)); /* 0x1247D8 */
    wr16(m, obj + 0x22u, 0xA000u);                                 /* 0x1247DE */
    wr16(m, obj + 0x20u, 0xA000u);                                 /* 0x1247E4 */
    wr16(m, obj + 0x42u, 0x0006u);                                 /* 0x1247EA */
    wr16(m, obj + 0x3Eu, 0u);                                      /* 0x1247F0 */
    wr16(m, obj + 0x3Au, 0u);                                      /* 0x1247F4 */
    wr32(m, obj, 0x00124800u);                                     /* 0x1247F8 */
    return 0;
}
