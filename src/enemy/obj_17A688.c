#include "obj_17A688.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x17A688: steady state of the palette-slot holder (0x17A664 latches it
 * after the first pass). Every frame it hands the five longs at +0x44.. to
 * 0x00183C (Pal_QueueRamp, real arguments (A0,A1,D0,D1,D2) = (+0x44,+0x48,
 * +0x4C,+0x50,+0x54)) while bit 3 of A5+0x4B26 is set. The claimed slot is
 * kept in +0x0C; a claim while a slot is still held releases the old one first
 * and decrements the +0x50 counter, and once that counter runs out the object
 * is freed. Bit 4 of A5-0x144C frees it immediately.
 *
 * The frame that never claims (A5+0x4B26 bit 3 clear) writes nothing at all,
 * which is why the 0x17A688 capture shows an unchanged object. */

int Obj_17A688_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A688: btst.b #4,-0x144C(A5) ; bne.w 0x17A6DC */
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)
        goto free_self;

    /* 0x17A692: tst.l (0xC,A0) ; bne.w 0x17A6CA */
    if (rd32(m, obj + 0x0Cu) != 0u) {
        /* 0x17A6CA / 0x17A6CE: release the slot held from the previous frame */
        wr16(m, rd32(m, obj + 0x0Cu), 0u);
        wr32(m, obj + 0x0Cu, 0u);                      /* 0x17A6D2 */
        /* 0x17A6D6: subq.l #1,(0x50,A0) ; bgt.b 0x17A680 */
        {
            int32_t n = (int32_t)rd32(m, obj + 0x50u) - 1;
            wr32(m, obj + 0x50u, (uint32_t)n);
            if (n > 0) {
                wr32(m, obj + 0x00u, 0x0017A688u);
                return 0;
            }
        }
        goto free_self;
    }

    /* 0x17A69A: bftst $4B26(A5){3:1} ; beq.b 0x17A680 */
    if ((rd16(m, F3_A5 + 0x4B26u) & 0x0008u) == 0u) {
        wr32(m, obj + 0x00u, 0x0017A688u);
        return 0;
    }

    /* 0x17A6A2..0x17A6B6: pea (0x54,A0) .. pea (0x44,A0) ; jsr 0x00183C */
    {
        uint32_t slot = Pal_QueueRamp(m,
                                      rd32(m, obj + 0x44u),
                                      rd32(m, obj + 0x48u),
                                      rd32(m, obj + 0x4Cu),
                                      rd32(m, obj + 0x50u),
                                      rd32(m, obj + 0x54u));
        /* 0x17A6C0: move.l D0,(0xC,A0) -- the claimed slot, or 0 */
        wr32(m, obj + 0x0Cu, slot);
    }
    wr32(m, obj + 0x00u, 0x0017A688u);                 /* 0x17A680 bra.b */
    return 0;

free_self:
    /* 0x17A6DC: tst.l (0xC,A0) ; beq.w 0x17A6EC */
    if (rd32(m, obj + 0x0Cu) != 0u)
        wr16(m, rd32(m, obj + 0x0Cu), 0u);
    Obj_Free(m, obj);                                  /* 0x17A6EE jsr 0x0105EC */
    return 0;
}
