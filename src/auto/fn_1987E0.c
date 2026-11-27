/* ROM 0x1987E0: object initialiser (A0 = obj). Resets the motion/anim state
 * (0x17A7FA), copies the parent's +0x50/+0x54 position words into +0x14/+0x18
 * and +0x50/+0x54, sets +0x44 = -1 and +0x22 = 4, spawns two 0x17A8D0 objects
 * (group 3, two groups, args 0x19BA0A and 0x19BA1A), then +0x6 = 1 and re-arms
 * with tick 0x198864. The `move.w #1,(0x6,A0) ; beq` can never branch taken, so
 * the 0x19886A block (four more spawns) is dead on every path. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x0017A7FA: clear the object's motion / animation fields. */
static void Fn1987E0_Reset(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0u);                        /* 0x17A7FA */
    wr16(m, obj + 0x18u, 0u);                        /* 0x17A7FE */
    wr16(m, obj + 0x20u, 0u);                        /* 0x17A802 */
    wr32(m, obj + 0x50u, 0u);                        /* 0x17A806 */
    wr32(m, obj + 0x54u, 0u);                        /* 0x17A80A */
    wr32(m, obj + 0x58u, 0u);                        /* 0x17A80E */
    wr16(m, obj + 0x16u, 0u);                        /* 0x17A812 */
    wr16(m, obj + 0x1Au, 0u);                        /* 0x17A816 */
    wr16(m, obj + 0x42u, 0u);                        /* 0x17A81A */
    wr16(m, obj + 0x5Cu, 0u);                        /* 0x17A81E */
    wr16(m, obj + 0x5Eu, 0u);                        /* 0x17A822 */
    wr16(m, obj + 0x60u, 0u);                        /* 0x17A826 */
    wr32(m, obj + 0x4Cu, 0xFFFFFFFFu);               /* 0x17A82A move.l #-0x1,(0x4c,A0) */
    wr16(m, obj + 0x1Eu, 0u);                        /* 0x17A832 */
    wr16(m, obj + 0x1Cu, 0u);                        /* 0x17A836 */
    wr8(m, obj + 0x22u, 0u);                         /* 0x17A83A */
    wr8(m, obj + 0x23u, 0u);                         /* 0x17A83E */
    wr8(m, obj + 0x34u, 0u);                         /* 0x17A842 */
    wr8(m, obj + 0x35u, 0u);                         /* 0x17A846 */
}

int Fn_1987E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent;
    uint16_t d0, d1;

    Fn1987E0_Reset(m, obj);                          /* 0x1987E0 jsr 0x0017a7fa */
    parent = rd32(m, obj + 0x08u);                    /* 0x1987E6 movea.l (0x8,A0),A4 */
    d0 = rd16(m, parent + 0x50u);                     /* 0x1987EA move.w (0x50,A4),D0w */
    d1 = rd16(m, parent + 0x54u);                     /* 0x1987EE move.w (0x54,A4),D1w */
    d0 = (uint16_t)(d0 + rd16(m, obj + 0x16u));       /* 0x1987F2 add.w (0x16,A0),D0w */
    d1 = (uint16_t)(d1 + rd16(m, obj + 0x1Au));       /* 0x1987F6 add.w (0x1a,A0),D1w */
    wr16(m, obj + 0x50u, d0);                         /* 0x1987FA move.w D0w,(0x50,A0) */
    wr16(m, obj + 0x54u, d1);                         /* 0x1987FE move.w D1w,(0x54,A0) */
    wr16(m, obj + 0x14u, d0);                         /* 0x198802 move.w D0w,(0x14,A0) */
    wr16(m, obj + 0x18u, d1);                         /* 0x198806 move.w D1w,(0x18,A0) */
    wr32(m, obj + 0x44u, 0xFFFFFFFFu);                /* 0x19880A move.l #-0x1,(0x44,A0) */
    wr8(m, obj + 0x22u, 0x04u);                       /* 0x198812 move.b #0x4,(0x22,A0) */

    /* 0x198818..0x19882C: push 0x19ba0a, #2, #3, #0x17a8d0 ; jsr 0x01057C */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BA0Au);
    /* 0x198836..0x19884A: the same with arg 0x19ba1a */
    r->d[0] = Obj_Spawn(m, obj, 0x0017A8D0u, 3u, 2u, 0x0019BA1Au);

    wr16(m, obj + 0x06u, 1u);                         /* 0x198854 move.w #0x1,(0x6,A0) */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | d1;           /* D1.w from 0x1987F6 */
    wr32(m, obj, 0x00198864u);                        /* 0x19885C move.l #0x198864,(A0) */
    return 0;                                         /* 0x198862 rts */
}
