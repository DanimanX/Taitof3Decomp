/* ROM 0x18ED80: delayed playfield clear & free tick (A0 = object).
 * Decrements word +0x06; while > 0 re-arms 0x18ED80. Otherwise clears two
 * playfield rects at 0x612000 (0x72CE4) and 0x612C00 (0x72D28) via
 * Video_ClearRect (0x0054AC) and frees self (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

int Fn_18ED80(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x18ED80 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                        /* 0x18ED84 */
        wr32(m, obj, 0x0018ED80u);                        /* 0x18ED78 */
        return 0;
    }

    Video_ClearRect(m, 0x00072CE4u, 0x00612000u);         /* 0x18ED86..0x18ED98 */
    Video_ClearRect(m, 0x00072D28u, 0x00612C00u);         /* 0x18ED9C..0x18EDAE */
    Obj_Free(m, obj);                                     /* 0x18EDB2..0x18EDBA */
    return 0;
}
