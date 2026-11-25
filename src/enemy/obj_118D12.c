#include "obj_118D12.h"
/* ROM 0x118D12: start of a 16-phase boss part script. The current phase's
 * record is chained in at +0xC (packed offsets, see 0x118D7C); the tick hands
 * over to the script stepper 0x118D66, which loads the next record into +0xC
 * (A1 pointer) and +0x6 (countdown), then parks on 0x118D7C. */
int Obj_118D12_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x118D12: move.l #0x118D1E,(0x3A,A0) -- phase table */
    wr32(m, obj + 0x3Au, 0x00118D1Eu);
    /* 0x118D1A: bra 0x118D66 -- no rts on this path */

    /* ROM 0x118D66: A1 = (0xC,A0) ; D0w = (A1)+ ; (0xC,A0) = A1 ; +0x6 = D0w */
    {
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        uint16_t d0 = rd16(m, a1);
        a1 += 2u;
        wr32(m, obj + 0x0Cu, a1);
        wr16(m, obj + 0x06u, d0);
    }
    /* ROM 0x118D74: move.l #0x118D7C,(A0) ; rts */
    wr32(m, obj + 0x00u, 0x00118D7Cu);
    return 0;
}

