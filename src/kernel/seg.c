/* Segment dispatcher (see seg.h): resume PC -> the group that holds it. */
#include "seg.h"

uint32_t Seg_Run(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    pc &= 0x00FFFFFFu;
    if (pc >= 0x0013F5B8u && pc < 0x0013F6A0u)
        return Seg_13F5B8(m, r, pc);
    if (pc >= 0x0013DAD4u && pc < 0x0013DE08u)
        return Seg_13DAD4(m, r, pc);
    if (pc >= 0x0013D69Au && pc < 0x0013D79Au)
        return Seg_13D69A(m, r, pc);
    return SEG_LEAVE;
}
