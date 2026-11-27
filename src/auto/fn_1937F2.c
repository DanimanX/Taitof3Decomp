/* ROM 0x1937F2: build a chain of 11 child objects. A0 = object: sets its
 * +0x16/+0x1A motion words, then spawns 11 objects of routine 0x193974 (group
 * 3, two groups) with arg 0..10 -- A0 stays the parent for all of them,
 * the +0x66/+0x62 links chain the children -- then re-arms A0 with
 * 0x19383C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_1937F2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a3 = obj;
    uint32_t a4 = obj;
    uint32_t d0 = 0;

    wr16(m, obj + 0x16u, 0xFFC0u);                    /* 0x1937F2 move.w #-0x40,(0x16,A0) */
    wr16(m, obj + 0x1Au, 0xFFF0u);                    /* 0x1937F8 move.w #-0x10,(0x1a,A0) */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0x000Au;      /* 0x1937FE move.w #0xa,D1w */
    r->d[2] = 0;                                      /* 0x193802 clr.l D2 */

    for (int i = 0; i < 11; i++) {                     /* 0x193808..0x193830 dbf D1w */
        /* push D2, #2, #3, #0x193974 ; jsr 0x01057C ; lea (0xc,SP),SP */
        d0 = Obj_Spawn(m, obj, 0x00193974u, 3u, 2u, (uint32_t)i); /* A0 stays the parent */
        wr32(m, a3 + 0x66u, d0);                       /* 0x193822 move.l D0,(0x66,A3) */
        a4 = d0;
        wr32(m, a4 + 0x62u, a3);                       /* 0x193828 move.l A3,(0x62,A4) */
        a3 = a4;                                       /* 0x19382C movea.l A4,A3 */
    }
    r->d[0] = d0;                                      /* 0x193822 leaves the last child */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0xFFFFu;       /* dbf leaves -1 in D1.w */
    r->d[2] = 0x0000000Bu;                             /* clr.l D2 + 11 addq.w #1 */

    wr32(m, obj, 0x0019383Cu);                         /* 0x193834 move.l #0x19383c,(A0) */
    return 0;                                          /* 0x19383A rts */
}
