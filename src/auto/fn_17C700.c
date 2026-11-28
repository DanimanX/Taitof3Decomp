/* ROM 0x17C700: `bra.w 0x17C662` (60 00 FF 60 at 0x17C700), so the whole
 * routine is the 0x17C662 fade and its Fn_ wrapper; nothing is duplicated
 * here. The capture work/ram/fnk_17C700 was taken on this entry and has the
 * same post state as fnk_17C662 (the shared body stores 0x0017C700 into (A0)
 * on the way out, so both entries re-arm to 0x17C700). */
#include "fn_auto.h"

int Fn_17C662(const f3mem_t *m, f3regs_t *r);

int Fn_17C700(const f3mem_t *m, f3regs_t *r)
{
    return Fn_17C662(m, r);                         /* 0x17C700 bra.w 0x17C662 */
}
