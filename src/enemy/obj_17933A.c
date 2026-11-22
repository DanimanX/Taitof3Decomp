#include "obj_17933A.h"

/* ROM 0x17933A: the argument at +0x0C becomes the record cursor (+0x44), its
 * first word (the record count) the countdown (+0x48), the step index (+0x4A)
 * starts at 0, and the tick vector moves to 0x179354. */
int Obj_17933A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x4Au, 0u);   /* clr.w (0x4a,A0) */

    /* LAB_0017934c */
    wr32(m, obj + 0x00u, 0x00179354u);
    return 0;
}
