/* ROM 0x162FE6: countdown tick clearing A5+0x4286 via 0x16AB58 (A0 = object). */
#include "fn_auto.h"

int Fn_162FE6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x162FE6 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                              /* 0x162FEA */
        wr32(m, obj, 0x00162FE6u);                       /* 0x162FDE */
        return 0;
    }

    r->d[0] &= 0xFFFFFF00u;                              /* 0x162FF4 move.b #0,D0 */
    wr8(m, F3_A5 + 0x4286u, 0u);                         /* 0x162FF8 -> 0x16AB58 */
    wr32(m, obj, 0x0016300Au);                           /* 0x163002 */
    return 0;
}
