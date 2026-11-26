/* ROM 0x13CB3A: start stage timed effect B indexed by table 0x13D61C[A5+0x32DC]. */
#include "fn_auto.h"
#include "../stage/stage_timeline.h"

int Fn_13CB3A(const f3mem_t *m, f3regs_t *r)
{
    int16_t tbl_idx = (int16_t)rd16(m, F3_A5 + 0x32DCu); /* 0x13CB40 */
    uint8_t id = rd8(m, 0x0013D61Cu + (uint32_t)(int32_t)tbl_idx); /* 0x13CB3A..0x13CB48 */

    Fx_StartB(m, id);                                    /* 0x13CB4E -> 0x0015EC */

    uint16_t idx = id & 7u;                              /* 0x0015FE..0x001602 */
    uint16_t q = id >> 3;
    int32_t dur = (int32_t)rd32(m, 0x000168C0u + 4u + 8u * (uint32_t)idx); /* 0x001608 */
    r->a[0] = 0x000168C0u;                               /* 0x0015F2 */
    r->d[0] = ((uint32_t)q << 16) | (uint32_t)idx;       /* 0x0015FE..0x001602 */
    if (dur > 0)                                         /* 0x00160C..0x001610 */
        r->d[1] = ((uint32_t)dur & 0xFFFF0000u) | (uint16_t)((uint16_t)dur + 1u);
    else
        r->d[1] = (uint32_t)dur;
    return 0;
}
