#include "obj_17A3F4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17A3F4: the entry tick of the 0x17A504 object. Installs frame list
 * 0x17A4A8 (index 0, {dur 2, spr 0x0005A17C}), runs that index's init, arms the
 * +0x06 wait with 1 and parks on 0x17A420. The wait body at 0x17A420 counts the
 * timer down on the following ticks and, once it expires, falls through 0x17A426
 * to the shared hand-over 0x17A44A. */

int Obj_17A3F4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17A3F4: frame list 0x17A4A8 -- {index 0, dur 2, spr 0x0005A17C, ...} */
    {
        uint32_t a1 = 0x0017A4A8u;
        uint16_t d0 = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, d0);
        wr32(m, obj + 0x3Au, a1);
        /* 0x17A404: movea.l (0x0,A1,D0w*8),A1 with A1 = 0x179A5C ; jsr (A1) */
        if (Obj_AnimInit(m, obj, d0) < 0)
            return -1;
    }

    /* 0x17A410: move.w #1,(0x6,A0) ; beq.b 0x17A426 (never taken) */
    wr16(m, obj + 0x06u, 0x0001u);
    wr32(m, obj + 0x00u, 0x0017A420u);          /* 0x17A418 */

    /* The first tick stops at 0x17A418: the capture (pair 0 of
     * work/ram/obj_17A3F4) keeps +0x06 at 1 and the +0x3A list untouched, so
     * the ROM's `beq.b 0x17A426` is taken here -- the wait body at 0x17A420 runs
     * on the NEXT tick, not as a fall-through of this one. */
    return 0;
}
