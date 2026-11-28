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
    if (pc >= 0x000B8404u && pc < 0x000B935Au)
        return Seg_0B8404(m, r, pc);
    if (pc >= 0x000B935Au && pc < 0x000B9682u)
        return Seg_0B935A(m, r, pc);
    if (pc >= 0x0019F504u && pc < 0x0019F684u)
        return Seg_19F504(m, r, pc);
    if (pc >= 0x0019F708u && pc < 0x0019FFF0u)
        return Seg_19F708(m, r, pc);
    if (pc >= 0x001A00E6u && pc < 0x001A043Cu)
        return Seg_1A00E6(m, r, pc);
    if (pc >= 0x001A043Cu && pc < 0x001A0942u)
        return Seg_1A043C(m, r, pc);
    if (pc >= 0x001A0942u && pc < 0x001A095Eu)
        return Seg_1A0942(m, r, pc);
    if (pc >= 0x001A095Eu && pc < 0x001A0A52u)
        return Seg_1A095E(m, r, pc);
    if (pc >= 0x001A0A52u && pc < 0x001A1282u)
        return Seg_1A0A52(m, r, pc);
    if (pc >= 0x001A1282u && pc < 0x001A1856u)
        return Seg_1A1282(m, r, pc);
    if (pc >= 0x001A1A26u && pc < 0x001A1BA6u)
        return Seg_1A1A26(m, r, pc);
    if (pc >= 0x001A1856u && pc < 0x001A1944u)
        return Seg_1A1856(m, r, pc);
    if (pc >= 0x001A1944u && pc < 0x001A1A26u)
        return Seg_1A1944(m, r, pc);
    return SEG_LEAVE;
}
