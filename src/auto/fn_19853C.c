/* ROM 0x19853C: wave spawner. Clears the common fields (0x17A7FA), folds the
 * owner's position (+0x50/+0x54 of A4 = (A0)+8) with this object's offsets
 * (+0x16/+0x1A, just cleared) into +0x50/+0x54/+0x14/+0x18, and then:
 *   - Y sum != 0: arms the one-tick wait 0x198584 and returns,
 *   - Y sum == 0: spawns three group-3 helpers (0x19A1A0 arg 0, 0x17A8D0
 *     arg 0x19BA3A, 0x17A8D0 arg 0x19BA9A), stores the last child in
 *     A5+0x4BB4 and arms 0x1985F6.
 * The beq at 0x1985EC tests the Z left by the 0x1057C at 0x1985DC, and that
 * helper ends with `move.l a2,d0` on a pointer into the object pool
 * (0x410060+), so Z is always clear: the 0x1985EE arm is taken on every call
 * and the block from 0x1985FC on is the continuation of the *next* state
 * 0x1985F6 (`subq.w #1,(0x6,A0) ; bgt 0x1985EE`, falling through to
 * 0x1985FC), not a path of this entry. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/obj_pool.h"

int Fn_19853C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint16_t y;

    Obj_17A7FA(m, obj);                              /* 0x19853C jsr 0x17A7FA */
    uint32_t a4 = rd32(m, obj + 0x08u);              /* 0x198542 */

    d0 = (d0 & 0xFFFF0000u) | rd16(m, a4 + 0x50u);   /* 0x198546 */
    d1 = (d1 & 0xFFFF0000u) | rd16(m, a4 + 0x54u);   /* 0x19854A */
    d0 = (d0 & 0xFFFF0000u)
       | (uint16_t)((uint16_t)d0 + rd16(m, obj + 0x16u)); /* 0x19854E */
    y = (uint16_t)((uint16_t)d1 + rd16(m, obj + 0x1Au));  /* 0x198552 */
    d1 = (d1 & 0xFFFF0000u) | y;

    wr16(m, obj + 0x50u, (uint16_t)d0);              /* 0x198556 */
    wr16(m, obj + 0x54u, (uint16_t)d1);              /* 0x19855A */
    wr16(m, obj + 0x14u, (uint16_t)d0);              /* 0x19855E */
    wr16(m, obj + 0x18u, (uint16_t)d1);              /* 0x198562 */
    wr32(m, obj + 0x44u, 0xFFFFFFFFu);               /* 0x198566 */
    wr8(m, obj + 0x22u, 3u);                         /* 0x19856E */
    wr16(m, obj + 0x06u, 1u);                        /* 0x198574 */

    /* 0x19857A beq.b 0x19858A: `move.w #1,(0x6,A0)` does not touch the CCR, so
     * this tests the Z of the 0x198552 add.w, i.e. "owner Y + offset == 0". */
    if (y != 0u) {
        wr32(m, obj + 0x00u, 0x00198584u);           /* 0x19857C */
        r->d[0] = d0;
        r->d[1] = d1;
        return 0;                                    /* 0x198582 rts */
    }

    /* 0x19858A..0x1985DC: every call is `pea arg ; pea 2 ; pea 3 ; pea fn`. */
    Obj_Spawn(m, obj, 0x0019A1A0u, 3u, 2u, 0x00000000u);   /* 0x19858A..0x19859C */
    d0 = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BA3Au); /* 0x1985A6..0x1985BA */
    wr32(m, F3_A5 + 0x4BB4u, d0);                          /* 0x1985C4 */
    d0 = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BA9Au); /* 0x1985C8..0x1985DC */

    wr16(m, obj + 0x06u, 1u);                              /* 0x1985E6 */
    wr32(m, obj + 0x00u, 0x001985F6u);                     /* 0x1985EE (beq never taken) */
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;                                              /* 0x1985F4 rts */
}
