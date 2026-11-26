/* ROM 0x121AC0: delayed palette upload tick (A0 = object).
 * Decrements +0x06 and re-arms 0x121AC0 while > 0. When <= 0, queues a
 * 0x20-word palette upload from A5+0x30E6 to 0x5E80 via Pal_QueueUpload
 * (0x001660), sets bit 11 of A5+0x30EE (A5+0x30EF |= 0x10), and advances
 * to 0x121AEE. */
#include "fn_auto.h"
#include "../video/palette.h"

int Fn_121AC0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x121AC0 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                  /* 0x121AC4 */
        wr32(m, obj, 0x00121AC0u);                           /* 0x121AB8 */
        return 0;
    }

    Pal_QueueUpload(m, rd32(m, F3_A5 + 0x30E6u), 0x5E80u, 0x0020u); /* 0x121AC6..0x121ADC */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x10u)); /* 0x121AE0 {11:1} */
    wr32(m, obj, 0x00121AEEu);                               /* 0x121AE6 */
    return 0;
}
