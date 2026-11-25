#include "obj_17AC7E.h"
#include "../objects/obj_pool.h"

/* ROM 0x17AC7E: script init (+0x42 from +0x0E, script 0x17ACFA) -> 0x17ACA2 */
int Obj_17AC7E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17AC7E: move.l (0xc,A0),D0 */
    uint32_t d0 = rd32(m, obj + 0x0cu);
    /* 0x17AC82: move.w D0w,(0x42,A0) */
    wr16(m, obj + 0x42u, (uint16_t)d0);
    /* 0x17AC86: movea.l #0x17acfa,A1 */
    uint32_t a1 = 0x0017ACFA;
    /* 0x17AC8C: move.l A1,(0x44,A0) */
    wr32(m, obj + 0x44u, a1);
    /* 0x17AC90: move.w (0x4,A1),(0x48,A0) */
    wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));
    /* 0x17AC96: clr.w (0x4a,A0) */
    wr16(m, obj + 0x4au, 0x0000u);
    /* 0x17AC9A: move.l #0x17aca2,(A0) */
    wr32(m, obj + 0x00u, 0x0017ACA2u);
    /* 0x17ACA0: rts */
    return 0;
}
