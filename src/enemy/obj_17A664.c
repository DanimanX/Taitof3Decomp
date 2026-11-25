#include "obj_17A664.h"

/* ROM 0x17A664: the palette-slot holder. The first pass moves the five longs
 * of the record at +0x0C (the two ramp sources and the brightness/step pair)
 * into +0x44..+0x57, drops the record pointer and latches 0x17A688. The steady
 * state hands them to 0x00183C (Pal_QueueRamp) while bit 3 of A5+0x4B26 is
 * set, keeps the returned slot in +0x0C and decrements the +0x50 counter; once
 * the counter runs out the held slot's first word is cleared and the object is
 * freed. Bit 4 of A5-0x144C frees it immediately. */

int Obj_17A664_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A664: movea.l (0xC,A0),A1 ; move.l (A1)+,(0x44..0x54,A0) x5 */
    {
        uint32_t a1 = rd32(m, obj + 0x0Cu);
        for (uint32_t off = 0x44u; off <= 0x54u; off += 4u) {
            wr32(m, obj + off, rd32(m, a1));
            a1 += 4u;
        }
        wr32(m, obj + 0x0Cu, 0u);                      /* 0x17A67C clr.l */
    }
    wr32(m, obj + 0x00u, 0x0017A688u);                 /* 0x17A680 */
    return 0;
}
